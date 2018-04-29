PostView : QAbstractScroll {
	var <stringColor, <editable=true, <colorScheme, <>mute = false;
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
		this.darkColorScheme;
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
		if (mute) { ^false };
		this.setProperty(\post, string.asString);
	}

	postln { arg string;
		if (mute) { ^false };
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

	colorScheme_ { |value|
    colorScheme = value;

    this.palette = QPalette.auto(colorScheme[\background], colorScheme[\background]);
		this.palette.base = colorScheme[\background];
    this.palette.baseText = colorScheme[\text];
    this.palette.highlight = colorScheme[\selectionBackground];
    this.palette.highlightText = colorScheme[\selectionText];
		this.setProperty(\successColor, colorScheme[\success]);
		this.setProperty(\warningColor, colorScheme[\warning]);
		this.setProperty(\errorColor, colorScheme[\error]);
  }
  darkColorScheme {
    this.colorScheme = (
      background: Color.new255(53, 53, 59).alpha_(0.9),
      text: Color.new255(216, 216, 216),
      selectionBackground: Color.new255(188, 210, 255),
      selectionText: Color.new255(42, 1, 0),
      success: Color.new255(139, 203, 95),
      warning: Color.new255(223, 161, 9),
      error: Color.new255(255, 117, 113)
    )
  }

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
