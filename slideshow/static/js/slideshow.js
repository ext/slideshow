String.prototype.repeat = function(num){
	return new Array( num + 1 ).join( this );
}

/* global states */
var delete_id = undefined; /* which slide is about to be deleted */

queue = function(){
	return {
		sorting: function(){
			/* test if queues exists on this page */
			if ( typeof(queues) == 'undefined' ){
				return;
			}

			/* queues is a global variable */
			for ( i in queues ){
				var id = queues[i];
				var other = queues.filter(function(x){ return x != id; });
				$(id).bind('updated', function() {
					var self = '#' + $(this).attr('id');
					var list = $(this).sortable('toArray');
					var n = list.length;

					/* show/hide warning about empty queue */
					if ( self == active ){
						if ( notice_visible && n > 0 ){
							notice_visible = false;
							$("#empty_notice").fadeOut("slow");
						}
						if ( !notice_visible && n == 0 ){
							notice_visible = true;
							$("#empty_notice").fadeIn("slow");
						}
					}

					/* notify server about update */
					$.ajax({
						type: "POST",
						url: "/slides/ajax/move",
						data: "queue=" + $(this).attr('id') + "&slides=" + list,
						error: function(x, status, error){
							alert(status + '\n' + error);
						},
					});
				});
				$(id).sortable({
					connectWith: other,
					placeholder: 'slide_placeholder',
					tolerance: 'pointer',
					distance: 10,
					update: function(){
						$(this).trigger('updated');
					},
				});
				$(id).disableSelection();
			}
		},
	};
}();

var slide = function(){
	var post = function(url, data, func){
		$.ajax({
			type: "POST",
			url: url,
			data: data,
			dataType: 'json',
			success: function(data){
				if ( !data.success ){
					alert(data.message);
					return;
				}
				return func(data);
			},
			error: function(x, status, error){
				alert(status + '\n' + error);
			},
		});
	};

	return {
		/* show a confirmation dialog and delete if user confirms (exception
		 * being if quick is true in which case it is deleted immediately) */
		delete: function(id, quick){
			delete_id = id;

			if ( quick ){
				slide.real_delete();
				return;
			}

			$("#delete_dialog img").attr('src', '/slides/show/' + id + '/800/600');
			$("#delete_dialog").dialog({
				modal: true,
				resizable: false,
				position: 'center',
				width: 834, /* 800 + 17 + 17 (padding) */
				height: 700,
				close: function(){
					delete_id = undefined;
				}
			});
		},

		/* delete slide without confirmation */
		real_delete: function(){
			$.ajax({
				type: "POST",
				url: "/slides/ajax/delete",
				data: {id: delete_id},
				dataType: 'json',
				success: function(data){
					if ( data['success'] ){
						$('#slide_' + delete_id).remove();
					} else {
						alert(data['message']);
					}
					$('#delete_dialog').dialog('close');
				},
				error: function(x, status, error){
					alert(status + '\n' + error);
					$('#delete_dialog').dialog('close');
				},
			});
		},

		activate: function(id, elem){
			return post("/slides/ajax/activate", {id: id}, function(data){
				parent = '#slide_' + id;
				$(parent).attr('class', data.class);
				console.log('actiaved');
			});
		},

		deactivate: function(id, elem){
			return post("/slides/ajax/deactivate", {id: id}, function(data){
				parent = '#slide_' + id;
				$(parent).attr('class', data.class);
				console.log('deactiaved');
			});
		},
	}
}();

var config = function(){
	function preview_transition(){
		var selected = $('.conf .transition :selected').val();
		$('#transition_preview').html('<img src="/transition/' + selected + '.gif" />');
	}

	return {
		preview_transition: preview_transition,
	};
}();

$(document).ready(function(){
	/* enable sorting on main page */
	queue.sorting();

	/* setup delete dialog buttons */
	$('#delete_cancel').bind('click', function(){
		$('#delete_dialog').dialog('close');
	});
	$('#delete_confirm').bind('click', slide.real_delete);

	/* fold slide upload fieldsets */
	var $f = $('.foldable');
	$f.foldable({
		collapsed: function(){
			/* all start collapsed but text assembler */
			return $(this).attr('id') != 'assembler_text';
		},
		expanded_html: '',
		collapsed_html: '',
		connected: $f,
	});

	/* preview transition during configuration */
	config.preview_transition();
	$('.conf .transition').change(function(){
		config.preview_transition();
	});
});

function update_browserstring(){
	provider = $("input[name='Database.Provider']").val();
	username = $("input[name='Database.Username']").val();
	password = $("input[name='Database.Password']").val();
	hostname = $("input[name='Database.Hostname']").val();
	name     = $("input[name='Database.Name']").val();

	/* build credential-part of the browserstring */
	credential = "";
	if ( username != "" ){
		credential = username;
		if ( password != "" ){
			credential += ":" + '*'.repeat(password.length);
		}
		credential += "@";
	}

	/* append a / to hostname if specified */
	if ( hostname != "" ){
		hostname += "/";
	}

	$('.browserstring').html(provider + "://" + credential + hostname + name);
}

function config_open(section){
	console.log('open section ' + section);

	/* reset current selection */
	$('#sidebar a').attr('class','');
	$('.conf fieldset').hide();

	$('#menu_'+section+' a').attr('class', 'selected');
	$('#conf_'+section).show();
}

var $text_preview_fields = [];
var text_preview_timer;
function text_preview(){
	clearTimeout(text_preview_timer);

	data = {}
	$text_preview_fields.each(function(){
		data[$(this).attr('name')] = $(this).val();
	});

	$('#assembler_text .preview img').attr('src', '/slides/preview?' + $.param(data));
}

function text_preview_init(){
	$text_preview_fields = $('#assembler_text .fields span').find('input, textarea');
	$text_preview_fields.each(function(){
		$(this).blur(text_preview);
		$(this).keydown(function(){
			clearTimeout(text_preview_timer);
			text_preview_timer = setTimeout(text_preview, 800);
		});
	});

	/* initial preview */
	text_preview();
}
