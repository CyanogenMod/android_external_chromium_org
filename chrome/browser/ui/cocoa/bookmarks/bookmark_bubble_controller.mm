// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "chrome/browser/ui/cocoa/bookmarks/bookmark_bubble_controller.h"

#include "base/mac/bundle_locations.h"
#include "base/mac/mac_util.h"
#include "base/sys_string_conversions.h"
#include "chrome/browser/bookmarks/bookmark_model.h"
#include "chrome/browser/bookmarks/bookmark_utils.h"
#import "chrome/browser/ui/cocoa/bookmarks/bookmark_button.h"
#import "chrome/browser/ui/cocoa/bookmarks/bookmark_cell_single_line.h"
#import "chrome/browser/ui/cocoa/browser_window_controller.h"
#import "chrome/browser/ui/cocoa/info_bubble_view.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/user_metrics.h"
#include "grit/generated_resources.h"
#include "ui/base/l10n/l10n_util_mac.h"

using content::UserMetricsAction;


// Simple class to watch for tab creation/destruction and close the bubble.
// Bridge between Chrome-style notifications and ObjC-style notifications.
class BookmarkBubbleNotificationBridge : public content::NotificationObserver {
 public:
  BookmarkBubbleNotificationBridge(BookmarkBubbleController* controller,
                                   SEL selector);
  virtual ~BookmarkBubbleNotificationBridge() {}
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details);
 private:
  content::NotificationRegistrar registrar_;
  BookmarkBubbleController* controller_;  // weak; owns us.
  SEL selector_;   // SEL sent to controller_ on notification.
};

BookmarkBubbleNotificationBridge::BookmarkBubbleNotificationBridge(
  BookmarkBubbleController* controller, SEL selector)
    : controller_(controller), selector_(selector) {
  // registrar_ will automatically RemoveAll() when destroyed so we
  // don't need to do so explicitly.
  registrar_.Add(this, content::NOTIFICATION_WEB_CONTENTS_CONNECTED,
                 content::NotificationService::AllSources());
  registrar_.Add(this, content::NOTIFICATION_WEB_CONTENTS_DESTROYED,
                 content::NotificationService::AllSources());
}

// At this time all notifications instigate the same behavior (go
// away) so we don't bother checking which notification came in.
void BookmarkBubbleNotificationBridge::Observe(
  int type,
  const content::NotificationSource& source,
  const content::NotificationDetails& details) {
  [controller_ performSelector:selector_ withObject:controller_];
}


// An object to represent the ChooseAnotherFolder item in the pop up.
@interface ChooseAnotherFolder : NSObject
@end

@implementation ChooseAnotherFolder
@end

@interface BookmarkBubbleController (PrivateAPI)
- (void)updateBookmarkNode;
- (void)fillInFolderList;
@end

@implementation BookmarkBubbleController

@synthesize node = node_;

+ (id)chooseAnotherFolderObject {
  // Singleton object to act as a representedObject for the "choose another
  // folder" item in the pop up.
  static ChooseAnotherFolder* object = nil;
  if (!object) {
    object = [[ChooseAnotherFolder alloc] init];
  }
  return object;
}

- (id)initWithParentWindow:(NSWindow*)parentWindow
                     model:(BookmarkModel*)model
                      node:(const BookmarkNode*)node
     alreadyBookmarked:(BOOL)alreadyBookmarked {
  if ((self = [super initWithWindowNibPath:@"BookmarkBubble"
                              parentWindow:parentWindow
                                anchoredAt:NSZeroPoint])) {
    model_ = model;
    node_ = node;
    alreadyBookmarked_ = alreadyBookmarked;
  }
  return self;
}

- (void)awakeFromNib {
  [super awakeFromNib];

  // Check if NSTextFieldCell supports the method. This check is in place as
  // only 10.6 and greater support the setUsesSingleLineMode method.
  // TODO(kushi.p): Remove this when the project hits a 10.6+ only state.
  NSTextFieldCell* nameFieldCell_ = [nameTextField_ cell];
  if ([nameFieldCell_
          respondsToSelector:@selector(setUsesSingleLineMode:)]) {
    [nameFieldCell_ setUsesSingleLineMode:YES];
  }
}

// If this is a new bookmark somewhere visible (e.g. on the bookmark
// bar), pulse it.  Else, call ourself recursively with our parent
// until we find something visible to pulse.
- (void)startPulsingBookmarkButton:(const BookmarkNode*)node  {
  while (node) {
    if ((node->parent() == model_->bookmark_bar_node()) ||
        (node == model_->other_node())) {
      pulsingBookmarkNode_ = node;
      NSValue *value = [NSValue valueWithPointer:node];
      NSDictionary *dict = [NSDictionary
                             dictionaryWithObjectsAndKeys:value,
                             bookmark_button::kBookmarkKey,
                             [NSNumber numberWithBool:YES],
                             bookmark_button::kBookmarkPulseFlagKey,
                             nil];
      [[NSNotificationCenter defaultCenter]
        postNotificationName:bookmark_button::kPulseBookmarkButtonNotification
                      object:self
                    userInfo:dict];
      return;
    }
    node = node->parent();
  }
}

- (void)stopPulsingBookmarkButton {
  if (!pulsingBookmarkNode_)
    return;
  NSValue *value = [NSValue valueWithPointer:pulsingBookmarkNode_];
  pulsingBookmarkNode_ = NULL;
  NSDictionary *dict = [NSDictionary
                         dictionaryWithObjectsAndKeys:value,
                         bookmark_button::kBookmarkKey,
                         [NSNumber numberWithBool:NO],
                         bookmark_button::kBookmarkPulseFlagKey,
                         nil];
  [[NSNotificationCenter defaultCenter]
        postNotificationName:bookmark_button::kPulseBookmarkButtonNotification
                      object:self
                    userInfo:dict];
}

// Close the bookmark bubble without changing anything.  Unlike a
// typical dialog's OK/Cancel, where Cancel is "do nothing", all
// buttons on the bubble have the capacity to change the bookmark
// model.  This is an IBOutlet-looking entry point to remove the
// dialog without touching the model.
- (void)dismissWithoutEditing:(id)sender {
  [self close];
}

- (void)windowWillClose:(NSNotification*)notification {
  // We caught a close so we don't need to watch for the parent closing.
  bookmark_observer_.reset(NULL);
  chrome_observer_.reset(NULL);
  [self stopPulsingBookmarkButton];
  [super windowWillClose:notification];
}

// Override -[BaseBubbleController showWindow:] to tweak bubble location and
// set up UI elements.
- (void)showWindow:(id)sender {
  NSWindow* window = [self window];  // Force load the NIB.
  NSWindow* parentWindow = self.parentWindow;
  BrowserWindowController* bwc =
      [BrowserWindowController browserWindowControllerForWindow:parentWindow];
  [bwc lockBarVisibilityForOwner:self withAnimation:NO delay:NO];

  InfoBubbleView* bubble = self.bubble;
  [bubble setArrowLocation:info_bubble::kTopRight];

  // Insure decent positioning even in the absence of a browser controller,
  // which will occur for some unit tests.
  NSPoint arrowTip = bwc ? [bwc bookmarkBubblePoint] :
      NSMakePoint([window frame].size.width, [window frame].size.height);
  arrowTip = [parentWindow convertBaseToScreen:arrowTip];
  NSPoint bubbleArrowTip = [bubble arrowTip];
  bubbleArrowTip = [bubble convertPoint:bubbleArrowTip toView:nil];
  arrowTip.y -= bubbleArrowTip.y;
  arrowTip.x -= bubbleArrowTip.x;
  [window setFrameOrigin:arrowTip];

  // Default is IDS_BOOKMARK_BUBBLE_PAGE_BOOKMARK; "Bookmark".
  // If adding for the 1st time the string becomes "Bookmark Added!"
  if (!alreadyBookmarked_) {
    NSString* title =
        l10n_util::GetNSString(IDS_BOOKMARK_BUBBLE_PAGE_BOOKMARKED);
    [bigTitle_ setStringValue:title];
  }

  [self fillInFolderList];

  // Ping me when things change out from under us.  Unlike a normal
  // dialog, the bookmark bubble's cancel: means "don't add this as a
  // bookmark", not "cancel editing".  We must take extra care to not
  // touch the bookmark in this selector.
  bookmark_observer_.reset(new BookmarkModelObserverForCocoa(
                               node_, model_,
                               self,
                               @selector(dismissWithoutEditing:)));
  chrome_observer_.reset(new BookmarkBubbleNotificationBridge(
                             self, @selector(dismissWithoutEditing:)));

  // Pulse something interesting on the bookmark bar.
  [self startPulsingBookmarkButton:node_];

  [parentWindow addChildWindow:window ordered:NSWindowAbove];
  [window makeKeyAndOrderFront:self];
  [self registerKeyStateEventTap];
}

- (void)close {
  [[BrowserWindowController browserWindowControllerForWindow:self.parentWindow]
      releaseBarVisibilityForOwner:self withAnimation:YES delay:NO];

  [super close];
}

// Shows the bookmark editor sheet for more advanced editing.
- (void)showEditor {
  [self ok:self];
  // Send the action up through the responder chain.
  [NSApp sendAction:@selector(editBookmarkNode:) to:nil from:self];
}

- (IBAction)edit:(id)sender {
  content::RecordAction(UserMetricsAction("BookmarkBubble_Edit"));
  [self showEditor];
}

- (IBAction)ok:(id)sender {
  [self stopPulsingBookmarkButton];  // before parent changes
  [self updateBookmarkNode];
  [self close];
}

// By implementing this, ESC causes the window to go away. If clicking the
// star was what prompted this bubble to appear (i.e., not already bookmarked),
// remove the bookmark.
- (IBAction)cancel:(id)sender {
  if (!alreadyBookmarked_) {
    // |-remove:| calls |-close| so don't do it.
    [self remove:sender];
  } else {
    [self ok:sender];
  }
}

- (IBAction)remove:(id)sender {
  [self stopPulsingBookmarkButton];
  bookmark_utils::RemoveAllBookmarks(model_, node_->url());
  content::RecordAction(UserMetricsAction("BookmarkBubble_Unstar"));
  node_ = NULL;  // no longer valid
  [self ok:sender];
}

// The controller is  the target of the pop up button box action so it can
// handle when "choose another folder" was picked.
- (IBAction)folderChanged:(id)sender {
  DCHECK([sender isEqual:folderPopUpButton_]);
  // It is possible that due to model change our parent window has been closed
  // but the popup is still showing and able to notify the controller of a
  // folder change.  We ignore the sender in this case.
  if (!self.parentWindow)
    return;
  NSMenuItem* selected = [folderPopUpButton_ selectedItem];
  ChooseAnotherFolder* chooseItem = [[self class] chooseAnotherFolderObject];
  if ([[selected representedObject] isEqual:chooseItem]) {
    content::RecordAction(
        UserMetricsAction("BookmarkBubble_EditFromCombobox"));
    [self showEditor];
  }
}

// The controller is the delegate of the window so it receives did resign key
// notifications. When key is resigned mirror Windows behavior and close the
// window.
- (void)windowDidResignKey:(NSNotification*)notification {
  NSWindow* window = [self window];
  DCHECK_EQ([notification object], window);
  if ([window isVisible]) {
    // If the window isn't visible, it is already closed, and this notification
    // has been sent as part of the closing operation, so no need to close.
    [self ok:self];
  }
}

// Look at the dialog; if the user has changed anything, update the
// bookmark node to reflect this.
- (void)updateBookmarkNode {
  if (!node_) return;

  // First the title...
  NSString* oldTitle = base::SysUTF16ToNSString(node_->GetTitle());
  NSString* newTitle = [nameTextField_ stringValue];
  if (![oldTitle isEqual:newTitle]) {
    model_->SetTitle(node_, base::SysNSStringToUTF16(newTitle));
    content::RecordAction(
        UserMetricsAction("BookmarkBubble_ChangeTitleInBubble"));
  }
  // Then the parent folder.
  const BookmarkNode* oldParent = node_->parent();
  NSMenuItem* selectedItem = [folderPopUpButton_ selectedItem];
  id representedObject = [selectedItem representedObject];
  if ([representedObject isEqual:[[self class] chooseAnotherFolderObject]]) {
    // "Choose another folder..."
    return;
  }
  const BookmarkNode* newParent =
      static_cast<const BookmarkNode*>([representedObject pointerValue]);
  DCHECK(newParent);
  if (oldParent != newParent) {
    int index = newParent->child_count();
    model_->Move(node_, newParent, index);
    content::RecordAction(UserMetricsAction("BookmarkBubble_ChangeParent"));
  }
}

// Fill in all information related to the folder pop up button.
- (void)fillInFolderList {
  [nameTextField_ setStringValue:base::SysUTF16ToNSString(node_->GetTitle())];
  DCHECK([folderPopUpButton_ numberOfItems] == 0);
  [self addFolderNodes:model_->root_node()
         toPopUpButton:folderPopUpButton_
           indentation:0];
  NSMenu* menu = [folderPopUpButton_ menu];
  NSString* title = [[self class] chooseAnotherFolderString];
  NSMenuItem *item = [menu addItemWithTitle:title
                                     action:NULL
                              keyEquivalent:@""];
  ChooseAnotherFolder* obj = [[self class] chooseAnotherFolderObject];
  [item setRepresentedObject:obj];
  // Finally, select the current parent.
  NSValue* parentValue = [NSValue valueWithPointer:node_->parent()];
  NSInteger idx = [menu indexOfItemWithRepresentedObject:parentValue];
  [folderPopUpButton_ selectItemAtIndex:idx];
}

@end  // BookmarkBubbleController


@implementation BookmarkBubbleController (ExposedForUnitTesting)

+ (NSString*)chooseAnotherFolderString {
  return l10n_util::GetNSStringWithFixup(
      IDS_BOOKMARK_BUBBLE_CHOOSER_ANOTHER_FOLDER);
}

// For the given folder node, walk the tree and add folder names to
// the given pop up button.
- (void)addFolderNodes:(const BookmarkNode*)parent
         toPopUpButton:(NSPopUpButton*)button
           indentation:(int)indentation {
  if (!model_->is_root_node(parent))  {
    NSString* title = base::SysUTF16ToNSString(parent->GetTitle());
    NSMenu* menu = [button menu];
    NSMenuItem* item = [menu addItemWithTitle:title
                                       action:NULL
                                keyEquivalent:@""];
    [item setRepresentedObject:[NSValue valueWithPointer:parent]];
    [item setIndentationLevel:indentation];
    ++indentation;
  }
  for (int i = 0; i < parent->child_count(); i++) {
    const BookmarkNode* child = parent->GetChild(i);
    if (child->is_folder() && child->IsVisible())
      [self addFolderNodes:child
             toPopUpButton:button
               indentation:indentation];
  }
}

- (void)setTitle:(NSString*)title parentFolder:(const BookmarkNode*)parent {
  [nameTextField_ setStringValue:title];
  [self setParentFolderSelection:parent];
}

// Pick a specific parent node in the selection by finding the right
// pop up button index.
- (void)setParentFolderSelection:(const BookmarkNode*)parent {
  // Expectation: There is a parent mapping for all items in the
  // folderPopUpButton except the last one ("Choose another folder...").
  NSMenu* menu = [folderPopUpButton_ menu];
  NSValue* parentValue = [NSValue valueWithPointer:parent];
  NSInteger idx = [menu indexOfItemWithRepresentedObject:parentValue];
  DCHECK(idx != -1);
  [folderPopUpButton_ selectItemAtIndex:idx];
}

- (NSPopUpButton*)folderPopUpButton {
  return folderPopUpButton_;
}

@end  // implementation BookmarkBubbleController(ExposedForUnitTesting)
