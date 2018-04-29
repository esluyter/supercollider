PostViewNew : QAbstractScroll {
	var <stringColor, <editable=true;
	classvar <all;

	*qtClass { ^'QcPostView' }

	*post { |str|
    all.do(_.post(str));
  }
  *postln { |str|
    all.do(_.postln(str));
  }

	*new { |parent, bounds|
		if (Quarks.isInstalled("ddwStatusBox")) {
			"Installed quark ddwStatusBox conflicts with PostView extensions to String. Please pick one to use and uninstall the other.".warn;
		};
    ^super.new(parent, bounds).init;
  }

	init {
		this.font_(Font.monospace);
		all = all.add(this);
		this.onClose_({ all.remove(this) });
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

	post { arg string;
		this.setProperty(\post, string.asString);
	}

	postln { arg string;
		this.setProperty(\post, string.asString ++ "\n");
	}

	clear {
		this.setProperty(\plainText, "");
	}

	maximumBlockCount {
		^this.getProperty(\maximumBlockCount);
	}

	maximumBlockCount_ { |count|
		this.setProperty(\maximumBlockCount, count);
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
