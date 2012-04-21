$(document).ready(function(){
		/* queue is a global variable */
		for ( i in queue ){
				var id = queue[i];
				var other = queue.filter(function(x){ return x != id; });
				$(id).bind('updated', function() {
						var self = '#' + $(this).attr('id');
						var list = $(this).sortable('toArray');
						var n = list.length;

						/* show/hide warning about empty queue */
						if ( self == active ){
								console.log('n: ' + n);
								if ( notice_visible && n > 0 ){
										console.log('hiding');
										notice_visible = false;
										$("#empty_notice").fadeOut("slow");
								}
								if ( !notice_visible && n == 0 ){
										console.log('showing');
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
});
