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
				delete: function(id){
						delete_id = id;
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

$(document).ready(function(){
		/* enable sorting on main page */
		queue.sorting();

		/* setup delete dialog buttons */
		$('#delete_cancel').bind('click', function(){
				$('#delete_dialog').dialog('close');
		});
		$('#delete_confirm').bind('click', function(){
				/* notify server about update */
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
		});
		
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
});
