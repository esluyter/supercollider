PostViewNew : QAbstractScroll {
	var <stringColor, <editable=true;

	*qtClass { ^'QcPostView' }

	*new { |parent, bounds|
    ^super.new(parent, bounds).init;
  }

	init {
		this.font_(Font.monospace);
		^this;
	}

	usesTabToFocusNextView_ { arg bool;
		this.setProperty( \tabChangesFocus, bool );
	}

	string {
		^this.getProperty( \plainText );
	}

	string_ { arg string;
		this.setProperty( \plainText, string );
	}

	font_ { arg argFont;
		font = argFont;
		this.setProperty( \textFont, font );
	}

	stringColor_ { arg color;
		stringColor = color;
		this.setProperty( \textColor, color );
	}

	background { ^this.palette.base }

	background_ { arg color; this.palette = this.palette.base_(color); }

	selectedString {
		var string;
		string = this.getProperty( \selectedString );
		if (string.size == 0) { string = this.getProperty( \currentLine ) };
		^string;
	}

	selectionStart {
		^this.getProperty( \selectionStart );
	}

	selectionSize {
		^this.getProperty( \selectionSize );
	}

	select { arg start, size;
		this.invokeMethod( \select, [start, size] );
	}

	tabWidth { ^this.getProperty( \tabStopWidth ); }

	tabWidth_ { arg pixels; this.setProperty( \tabStopWidth, pixels ); }
}
