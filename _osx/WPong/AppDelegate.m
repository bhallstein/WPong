#import "AppDelegate.h"
#include "W.h"
#include "PongState.h"

@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	W::createWindow(W::size(800,600), "WPong");
	W::pushState(new PongState);
	W::start();
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
	// Insert code here to tear down your application
}

@end
