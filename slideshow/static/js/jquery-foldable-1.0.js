(function($) {
		var update_link = function(){
				var $this = $(this);
				var data = $this.data('foldable');
				var marker = data.collapsed ? data.collapsed_html  : data.expanded_html;
				var cls    = data.collapsed ? data.collapsed_class : data.expanded_class;
				$this.find('.folding').html(marker).attr('class', 'folding ' + cls).disableSelection();
		};

		var toggle = function(propagate){
				/* collapse others */
				if ( typeof(propagate) == 'undefined' || propagate ){
						$(this).data('foldable').connected.each(function(){
								$(this).foldable('hide', false);
						});
				}

				var $this = $(this);
				var data = $this.data('foldable');
				$this.children().not('legend').slideToggle('fast');
				
				/* update state */
				data.collapsed = !data.collapsed;
				update_link.apply(this);
				$this.data('foldable', data);

				return false;
		}

		var methods = {
				init: function(options){
						return this.each(function(){
								var id = $(this).attr('id');
								var s = $.extend(true, {id: id}, $.fn.foldable.defaults, options);

								/* remove self from connected */
								s.connected = s.connected.filter(function(){ return $(this).attr('id') != id });

								/* if collapsed is function evaluate it */
								if ( typeof(s.collapsed) == 'function' ){
										s.collapsed = s.collapsed();
								}
								
								/* store settings */
								$(this).data('foldable', s);

								/* initial collapse */
								if ( s.collapsed ){
										$(this).children().not('legend').hide();
								};

								update_link.apply(this);
						});
				},

				toggle: toggle,
				
				show: function(propagate){
						return this.each(function(){
								var $this = $(this);
								var data = $this.data('foldable');
								if ( data.collapsed ){
										toggle.apply(this, [propagate]);
								}
								return false;
						});
				},

				hide: function(propagate){
						return this.each(function(){
								var $this = $(this);
								var data = $this.data('foldable');
								if ( !data.collapsed ){
										toggle.apply(this, [propagate]);
								}
								return false;
						});
				},
		};

    $.fn.foldable = function(method) {
				if ( methods[method] ) {
						return methods[method].apply( this, Array.prototype.slice.call( arguments, 1 ));
				} else if ( typeof method === 'object' || ! method ) {
						return methods.init.apply( this, arguments );
				} else {
						$.error( 'Method ' +  method + ' does not exist on jQuery.foldable' );
				}
		};

		$.foldable = {
				collapsed: false,
		};

    $.fn.foldable.defaults = {
				collapsed       : false,
				collapsed_html  : '&#x25be',
				collapsed_class : 'fold-collapsed',
				expanded_html   : '&#x25b4',
				expanded_class  : 'fold-expanded',
				connected       : [],
    };
})(jQuery);
