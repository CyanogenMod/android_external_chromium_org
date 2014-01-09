// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

/**
 * Group of progress item in the progress center panels.
 *
 * This is responsible for generating the summarized item and managing lifetime
 * of error items.
 * @param {string} name Name of the group.
 * @constructor
 */
function ProgressCenterItemGroup(name) {
  /**
   * Name of the group.
   * @type {string}
   */
  this.name = name;

  /**
   * State of the group.
   * @type {ProgressCenterItemGroup.State}
   * @private
   */
  this.state_ = ProgressCenterItemGroup.State.EMPTY;

  /**
   * Items that are progressing, or completed but still animated.
   * Key is item ID.
   * @type {Object.<string, ProgressCenterItem>}
   * @private
   */
  this.items_ = {};

  /**
   * Set of animated state of items. Key is item ID and value is whether the
   * item is animated or not.
   * @type {Object.<string, boolean>}
   * @private
   */
  this.animated_ = {};

  /**
   * Last summarized item.
   * @type {ProgressCenterItem}
   * @private
   */
  this.summarizedItem_ = null;

  /**
   * Whether the summarized item is animated or not.
   * @type {boolean}
   * @private
   */
  this.summarizedItemAnimated_ = false;

  /**
   * Total maximum progress value of items already completed and removed from
   * this.items_.
   * @type {number}
   * @private
   */
  this.totalProgressMax_ = 0;

  /**
   * Total progress value of items already completed and removed from
   * this.items_.
   * @type {number}
   * @private
   */
  this.totalProgressValue_ = 0;

  Object.seal(this);
}

/**
 * State of ProgressCenterItemGroup.
 * @enum {string}
 * @const
 */
ProgressCenterItemGroup.State = Object.freeze({
  // Group has no items.
  EMPTY: 'empty',
  // Group has at least 1 progressing item.
  ACTIVE: 'active',
  // Group has no progressing items but still shows error items.
  INACTIVE: 'inactive'
});

/**
 * Makes the summarized item for the groups.
 *
 * When a group has only error items, getSummarizedItem of the item returns
 * null. Basically the first result of the groups that the progress center panel
 * contains is used as a summarized item. But If all the group returns null, the
 * progress center panel generates the summarized item by using the method.
 *
 * @param {Array.<ProgressCenterItemGroup>} var_groups List of groups.
 * @return {ProgressCenterItem} Summarized item.
 */
ProgressCenterItemGroup.getSummarizedErrorItem = function(var_groups) {
  var groups = Array.prototype.slice.call(arguments);
  var errorItems = [];
  for (var i = 0; i < groups.length; i++) {
    for (var id in groups[i].items_) {
      var item = groups[i].items_[id];
      if (item.state === ProgressItemState.ERROR)
        errorItems.push(item);
    }
  }
  if (errorItems.length === 0)
    return null;

  if (errorItems.length === 1)
    return errorItems[0].clone();

  var item = new ProgressCenterItem();
  item.state = ProgressItemState.ERROR;
  item.message = strf('ERROR_PROGRESS_SUMMARY_PLURAL',
                      errorItems.length);
  return item;
};

/**
 * Obtains Whether the item should be animated or not.
 * @param {boolean} previousAnimated Whether the item is previously animated or
 *     not.
 * @param {ProgressCenterItem} previousItem Item before updating.
 * @param {ProgressCenterItem} item New item.
 * @return {boolean} Whether the item should be animated or not.
 * @private
 */
ProgressCenterItemGroup.shouldAnimate_ = function(
    previousAnimated, previousItem, item) {
  if (!previousItem)
    return false;
  if (!item)
    return false;
  if (previousItem.progressRateInPercent < item.progressRateInPercent)
    return true;
  if (previousAnimated &&
      previousItem.progressRateInPercent === item.progressRateInPercent)
    return true;
  return false;
};

ProgressCenterItemGroup.prototype = {
  /**
   * @return {ProgressCenterItemGroup.State} State of the group.
   */
  get state() {
    return this.state_;
  },

  /**
   * @return {number} Number of error items that the group contains.
   */
  get numErrors() {
    var result = 0;
    for (var id in this.items_) {
      if (this.items_[id].state === ProgressItemState.ERROR)
        result++;
    }
    return result;
  }
};

/**
 * Obtains the progressing (or completed but animated) item.
 *
 * @param {string} id Item ID.
 * @return {ProgressCenterItem} Item having the ID.
 */
ProgressCenterItemGroup.prototype.getItem = function(id) {
  return this.items_[id] || null;
};

/**
 * Obtains whether the item should be animated or not.
 * @param {string} id Item ID.
 * @return {boolean} Whether the item should be animated or not.
 */
ProgressCenterItemGroup.prototype.isAnimated = function(id) {
  return !!this.animated_[id];
};

/**
 * Obtains whether the summarized item should be animated or not.
 * @return {boolean} Whether the summarized item should be animated or not.
 */
ProgressCenterItemGroup.prototype.isSummarizedAnimated = function() {
  return this.summarizedItemAnimated_;
};

/**
 * Starts item update.
 * Marks the given item as updating.
 * @param {ProgressCenterItem} item Item containing updated information.
 */
ProgressCenterItemGroup.prototype.update = function(item) {
  // If the group is inactive, go back to the empty state.
  this.endInactive();

  // Compares the current state and the new state to check if the update is
  // valid or not.
  var previousItem = this.items_[item.id];
  switch (item.state) {
    case ProgressItemState.ERROR:
      if (previousItem && previousItem.state !== ProgressItemState.PROGRESSING)
        return;
      if (this.state_ === ProgressCenterItemGroup.State.EMPTY)
        this.state_ = ProgressCenterItemGroup.State.INACTIVE;
      this.items_[item.id] = item.clone();
      this.animated_[item.id] = false;
      break;

    case ProgressItemState.PROGRESSING:
    case ProgressItemState.COMPLETED:
      if ((!previousItem && item.state === ProgressItemState.COMPLETED) ||
          (previousItem &&
           previousItem.state !== ProgressItemState.PROGRESSING))
        return;
      if (this.state_ === ProgressCenterItemGroup.State.EMPTY)
        this.state_ = ProgressCenterItemGroup.State.ACTIVE;
      this.items_[item.id] = item.clone();
      this.animated_[item.id] = ProgressCenterItemGroup.shouldAnimate_(
          !!this.animated_[item.id],
          previousItem,
          item);
      if (!this.animated_[item.id])
        this.completeItemAnimation(item.id);
      break;

    case ProgressItemState.CANCELED:
      if (!previousItem ||
          previousItem.state !== ProgressItemState.PROGRESSING)
        return;
      delete this.items_[item.id];
      this.animated_[item.id] = false;
      this.summarizedItem_ = null;
  }

  // Update the internal summarized item cache.
  var previousSummarizedItem = this.summarizedItem_;
  this.summarizedItem_ = this.getSummarizedItem(0);
  this.summarizedItemAnimated_ = ProgressCenterItemGroup.shouldAnimate_(
      !!this.summarizedItemAnimated_,
      previousSummarizedItem,
      this.summarizedItem_);
  if (!this.summarizedItemAnimated_)
    this.completeSummarizedItemAnimation();
};

/**
 * Notifies the end of the item's animation to the group.
 * If all the items except error items completes, the group enter the inactive
 * state.
 * @param {string} id Item ID.
 */
ProgressCenterItemGroup.prototype.completeItemAnimation = function(id) {
  if (this.state_ !== ProgressCenterItemGroup.State.ACTIVE)
    return;

  this.animated_[id] = false;
  if (this.items_[id].state === ProgressItemState.COMPLETED) {
    this.totalProgressValue_ += (this.items_[id].progressValue || 0.0);
    this.totalProgressMax_ += (this.items_[id].progressMax || 0.0);
    delete this.items_[id];
    this.tryEndActive_();
  }
};

/**
 * Notifies the end of the summarized item's animation.
 * This may update summarized view. (1 progressing + 1 error -> 1 error)
 */
ProgressCenterItemGroup.prototype.completeSummarizedItemAnimation = function() {
  this.summarizedItemAnimated_ = false;
  this.tryEndActive_();
};

/**
 * Obtains the summary of the set.
 * @param {number} numOtherErrors Number of errors contained by other groups.
 * @return {ProgressCenterItem} Item.
 */
ProgressCenterItemGroup.prototype.getSummarizedItem =
    function(numOtherErrors) {
  if (this.state_ === ProgressCenterItemGroup.State.EMPTY ||
      this.state_ === ProgressCenterItemGroup.State.INACTIVE)
    return null;

  var summarizedItem = new ProgressCenterItem();
  summarizedItem.progressMax += this.totalProgressMax_;
  summarizedItem.progressValue += this.totalProgressValue_;
  var progressingItems = [];
  var errorItems = [];
  var numItems = 0;

  for (var id in this.items_) {
    var item = this.items_[id];
    numItems++;

    // Count states.
    switch (item.state) {
      case ProgressItemState.PROGRESSING:
      case ProgressItemState.COMPLETED:
        progressingItems.push(item);
        break;
      case ProgressItemState.ERROR:
        errorItems.push(item);
        continue;
    }

    // If all of the progressing items have the same type, then use
    // it. Otherwise use TRANSFER, since it is the most generic.
    if (summarizedItem.type === null)
      summarizedItem.type = item.type;
    else if (summarizedItem.type !== item.type)
      summarizedItem.type = ProgressItemType.TRANSFER;

    // Sum up the progress values.
    summarizedItem.progressMax += item.progressMax;
    summarizedItem.progressValue += item.progressValue;
  }

  // Returns 1 item.
  if (progressingItems.length === 1 &&
      errorItems.length + numOtherErrors === 0) {
    summarizedItem.id = progressingItems[0].id;
    summarizedItem.cancelCallback = progressingItems[0].cancelCallback;
    summarizedItem.message = progressingItems[0].message;
    summarizedItem.state = progressingItems[0].state;
    return summarizedItem;
  }

  // Returns integrated items.
  if (progressingItems.length > 0) {
    var messages = [];
    switch (summarizedItem.type) {
      case ProgressItemType.COPY:
        messages.push(str('COPY_PROGRESS_SUMMARY'));
        break;
      case ProgressItemType.MOVE:
        messages.push(str('MOVE_PROGRESS_SUMMARY'));
        break;
      case ProgressItemType.DELETE:
        messages.push(str('DELETE_PROGRESS_SUMMARY'));
        break;
      case ProgressItemType.ZIP:
        messages.push(str('ZIP_PROGRESS_SUMMARY'));
        break;
      case ProgressItemType.TRANSFER:
        messages.push(str('TRANSFER_PROGRESS_SUMMARY'));
        break;
    }
    var numErrors = errorItems.length + numOtherErrors;
    if (numErrors === 1)
      messages.push(str('ERROR_PROGRESS_SUMMARY'));
    else if (numErrors > 1)
      messages.push(strf('ERROR_PROGRESS_SUMMARY_PLURAL', numErrors));
    summarizedItem.summarized = true;
    summarizedItem.message = messages.join(' ');
    summarizedItem.state = ProgressItemState.PROGRESSING;
    return summarizedItem;
  }

  // Retruns error.
  if (errorItems.length > 0)
    return null;

  // Returns complete items.
  summarizedItem.state = ProgressItemState.COMPLETED;
  return summarizedItem;
};

/**
 * Goes back to the EMPTY state from the INACTIVE state. Removes all the items.
 * If the current state is not the INACTIVE, nothing happens.
 */
ProgressCenterItemGroup.prototype.endInactive = function() {
  if (this.state_ !== ProgressCenterItemGroup.State.INACTIVE)
    return;
  this.items_ = {};
  this.animated_ = {};
  this.summarizedItem_ = null;
  this.summarizedItemAnimated_ = false;
  this.totalProgressValue_ = 0.0;
  this.totalProgressMax_ = 0.0;
  this.state_ = ProgressCenterItemGroup.State.EMPTY;
};

/**
 * Ends active state if there is no progressing and animated items.
 * @private
 */
ProgressCenterItemGroup.prototype.tryEndActive_ = function() {
  if (this.state_ !== ProgressCenterItemGroup.State.ACTIVE ||
      this.summarizedItemAnimated_)
    return;
  var hasError = false;
  for (var id in this.items_) {
    // If there is non-error item (progressing, or completed but still
    // animated), we should stay the active state.
    if (this.items_[id].state !== ProgressItemState.ERROR)
      return;
    hasError = true;
  }
  this.state_ = ProgressCenterItemGroup.State.INACTIVE;
  if (!hasError)
    this.endInactive();
};
