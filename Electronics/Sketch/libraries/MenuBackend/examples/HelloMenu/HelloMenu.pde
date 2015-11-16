#include <MenuBackend.h>

/*
	This is the structure of the modelled menu
	
	Settings
		Pin
		Debug
	Options
		Delay (D)
			100 ms
			200 ms
			300 ms
			400 ms
*/

//this controls the menu backend and the event generation
MenuBackend menu = MenuBackend(menuUseEvent,menuChangeEvent);
	//beneath is list of menu items needed to build the menu
	MenuItem settings = MenuItem("Settings");
		MenuItem pin = MenuItem("Pin");
		MenuItem debug = MenuItem("Debug");
	MenuItem options = MenuItem("Options");
		MenuItem setDelay = MenuItem("Delay",'D');
			MenuItem d100 = MenuItem("100 ms");
			MenuItem d200 = MenuItem("200 ms");
			MenuItem d300 = MenuItem("300 ms");
			MenuItem d400 = MenuItem("400 ms");
	
//this function builds the menu and connects the correct items together
void menuSetup()
{
	Serial.println("Setting up menu...");
	//add the file menu to the menu root
	menu.getRoot().add(settings); 
		//setup the settings menu item
		settings.addRight(pin);
			//we want looping both up and down
			pin.addBefore(debug);
			pin.addAfter(debug);
			debug.addAfter(pin);
			//we want a left movement to pint to settings from anywhere
			debug.addLeft(settings);
			pin.addLeft(settings);
	settings.addBefore(options);
	settings.addAfter(options);
		options.addRight(setDelay);
			setDelay.addLeft(options);
			setDelay.addRight(d100);
				d100.addBefore(d100); //loop to d400 
				d100.addAfter(d200);
				d200.addAfter(d300);
				d300.addAfter(d400);
				d400.addAfter(d100); //loop back to d100
				//we want left to always be bak to delay
				d100.addLeft(setDelay);
				d200.addLeft(setDelay);
				d300.addLeft(setDelay);
				d400.addLeft(setDelay);
	options.addAfter(options);
}

/*
	This is an important function
	Here all use events are handled
	
	This is where you define a behaviour for a menu item
*/
void menuUseEvent(MenuUseEvent used)
{
	Serial.print("Menu use ");
	Serial.println(used.item.getName());
	if (used.item == setDelay) //comparison agains a known item
	{
		Serial.println("menuUseEvent found Dealy (D)");
	}
}

/*
	This is an important function
	Here we get a notification whenever the user changes the menu
	That is, when the menu is navigated
*/
void menuChangeEvent(MenuChangeEvent changed)
{
	Serial.print("Menu change ");
	Serial.print(changed.from.getName());
	Serial.print(" ");
	Serial.println(changed.to.getName());
}

void setup()
{
	Serial.begin(9600);
	
	menuSetup();
	Serial.println("Starting navigation:\r\nUp: w   Down: s   Left: a   Right: d   Use: e");
}

void loop()
{
	if (Serial.available()) {
		byte read = Serial.read();
		switch (read) {
			case 'w': menu.moveUp(); break;
			case 's': menu.moveDown(); break;
			case 'd': menu.moveRight(); break;
			case 'a': menu.moveLeft(); break;
			case 'e': menu.use(); break;
		}
	}
}


