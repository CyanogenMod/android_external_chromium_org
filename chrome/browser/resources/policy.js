// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

cr.define('policy', function() {
  /**
   * A box that shows the status of cloud policy for a device or user.
   * @constructor
   * @extends {HTMLFieldSetElement}
   */
  var StatusBox = cr.ui.define(function() {
    var node = $('status-box-template').cloneNode(true);
    node.removeAttribute('id');
    return node;
  });

  StatusBox.prototype = {
    // Set up the prototype chain.
    __proto__: HTMLFieldSetElement.prototype,

    /**
     * Initialization function for the cr.ui framework.
     */
    decorate: function() {
    },

    /**
     * Populate the box with the given cloud policy status.
     * @param {string} scope The policy scope, either "device" or "user".
     * @param {Object} status Dictionary with information about the status.
     */
    initialize: function(scope, status) {
      if (scope == 'device') {
        // For device policy, set the appropriate title and populate the topmost
        // status item with the domain the device is enrolled into.
        this.querySelector('.legend').textContent =
            loadTimeData.getString('statusDevice');
        var domain = this.querySelector('.domain');
        domain.textContent = status.domain;
        domain.parentElement.hidden = false;
      } else {
        // For user policy, set the appropriate title and populate the topmost
        // status item with the username that policies apply to.
        this.querySelector('.legend').textContent =
            loadTimeData.getString('statusUser');
        // Populate the topmost item with the username.
        var username = this.querySelector('.username');
        username.textContent = status.username;
        username.parentElement.hidden = false;
      }
      // Populate all remaining items.
      this.querySelector('.client-id').textContent = status.clientId || '';
      this.querySelector('.time-since-last-refresh').textContent =
          status.timeSinceLastRefresh || '';
      this.querySelector('.refresh-interval').textContent =
          status.refreshInterval || '';
      this.querySelector('.status').textContent = status.status || '';
    },
  };

  /**
   * A single policy's entry in the policy table.
   * @constructor
   * @extends {HTMLTableSectionElement}
   */
  var Policy = cr.ui.define(function() {
    var node = $('policy-template').cloneNode(true);
    node.removeAttribute('id');
    return node;
  });

  Policy.prototype = {
    // Set up the prototype chain.
    __proto__: HTMLTableSectionElement.prototype,

    /**
     * Initialization function for the cr.ui framework.
     */
    decorate: function() {
      this.updateToggleExpandedValueText_();
      this.querySelector('.toggle-expanded-value').addEventListener(
          'click', this.toggleExpandedValue_.bind(this));
    },

    /**
     * Populate the table columns with information about the policy name, value
     * and status.
     * @param {string} name The policy name.
     * @param {Object} value Dictionary with information about the policy value.
     * @param {boolean} unknown Whether the policy name is not recognized.
     * @param {boolean} includeStatus Whether the table has a status column.
     */
    initialize: function(name, value, unknown, includeStatus) {
      this.name = name;
      this.unset = !value;

      // Populate the name column.
      this.querySelector('.name').textContent = name;

      // Populate the remaining columns with policy scope, level and value if a
      // value has been set. Otherwise, leave them blank.
      if (value) {
        this.querySelector('.scope').textContent =
            loadTimeData.getString(value.scope == 'user' ?
                'scopeUser' : 'scopeDevice');
        this.querySelector('.level').textContent =
            loadTimeData.getString(value.level == 'recommended' ?
                'levelRecommended' : 'levelMandatory');
        this.querySelector('.value').textContent = value.value;
        this.querySelector('.expanded-value').textContent = value.value;
      }

      if (includeStatus) {
        // Populate the status column.
        var status;
        if (!value) {
          // If the policy value has not been set, show an error message.
          status = loadTimeData.getString('unset');
        } else if (unknown) {
          // If the policy name is not recognized, show an error message.
          status = loadTimeData.getString('unknown');
        } else if (value.error) {
          // If an error occurred while parsing the policy value, show the error
          // message.
          status = value.error;
        } else {
          // Otherwise, indicate that the policy value was parsed correctly.
          status = loadTimeData.getString('ok');
        }
        this.querySelector('.status').textContent = status;
      } else {
        // Remove status column.
        this.querySelector('.status-container').remove();
        this.querySelector('.expanded-value').setAttribute('colspan', 4);
      }
    },

    /**
     * Check the table columns for overflow. Most columns are automatically
     * elided when overflow occurs. The only action required is to add a tooltip
     * that shows the complete content. The value column is an exception. If
     * overflow occurs here, the contents is replaced with a link that toggles
     * the visibility of an additional row containing the complete value.
     */
    checkOverflow: function() {
      // Set a tooltip on all overflowed columns except the value column.
      var divs = this.querySelectorAll('div.elide');
      for (var i = 0; i < divs.length; i++) {
        var div = divs[i];
        div.title = div.offsetWidth < div.scrollWidth ? div.textContent : '';
      }

      // Cache the width of the value column's contents when it is first shown.
      // This is required to be able to check whether the contents would still
      // overflow the column once it has been hidden and replaced by a link.
      var valueContainer = this.querySelector('.value-container');
      if (valueContainer.valueWidth == undefined) {
        valueContainer.valueWidth =
            valueContainer.querySelector('.value').offsetWidth;
      }

      // Determine whether the contents of the value column overflows. The
      // visibility of the contents, replacement link and additional row
      // containing the complete value that depend on this are handled by CSS.
      if (valueContainer.offsetWidth < valueContainer.valueWidth)
        this.classList.add('has-overflowed-value');
      else
        this.classList.remove('has-overflowed-value');
    },

    /**
     * Update the text of the link that toggles the visibility of an additional
     * row containing the complete policy value, depending on the toggle state.
     * @private
     */
    updateToggleExpandedValueText_: function(event) {
      this.querySelector('.toggle-expanded-value').textContent =
          loadTimeData.getString(
              this.classList.contains('show-overflowed-value') ?
                  'hideExpandedValue' : 'showExpandedValue');
    },

    /**
     * Toggle the visibility of an additional row containing the complete policy
     * value.
     * @private
     */
    toggleExpandedValue_: function() {
      this.classList.toggle('show-overflowed-value');
      this.updateToggleExpandedValueText_();
    },
  };

  /**
   * A table of policies and their values.
   * @constructor
   * @extends {HTMLTableSectionElement}
   */
  var PolicyTable = cr.ui.define('tbody');

  PolicyTable.prototype = {
    // Set up the prototype chain.
    __proto__: HTMLTableSectionElement.prototype,

    /**
     * Initialization function for the cr.ui framework.
     */
    decorate: function() {
      this.policies_ = {};
      this.filterPattern_ = '';
      window.addEventListener('resize', this.checkOverflow_.bind(this));
    },

    /**
     * Initialize the list of all known policies.
     * @param {Object} names Dictionary containing all known policy names.
     */
    setPolicyNames: function(names) {
      this.policies_ = names;
      this.setPolicyValues({});
    },

    /**
     * Populate the table with the currently set policy values and any errors
     * detected while parsing these.
     * @param {Object} values Dictionary containing the current policy values.
     */
    setPolicyValues: function(values) {
      // Remove all policies from the table.
      var policies = this.getElementsByTagName('tbody');
      while (policies.length > 0)
        this.removeChild(policies.item(0));

      // First, add known policies whose value is currently set.
      var unset = [];
      for (var name in this.policies_) {
        if (name in values)
          this.setPolicyValue_(name, values[name], false);
        else
          unset.push(name);
      }

      // Second, add policies whose value is currently set but whose name is not
      // recognized.
      for (var name in values) {
        if (!(name in this.policies_))
          this.setPolicyValue_(name, values[name], true);
      }

      // Finally, add known policies whose value is not currently set.
      for (var i = 0; i < unset.length; i++)
        this.setPolicyValue_(unset[i], undefined, false);

      // Filter the policies.
      this.filter();
    },

    /**
     * Set the filter pattern. Only policies whose name contains |pattern| are
     * shown in the policy table. The filter is case insensitive. It can be
     * disabled by setting |pattern| to an empty string.
     * @param {string} pattern The filter pattern.
     */
    setFilterPattern: function(pattern) {
      this.filterPattern_ = pattern.toLowerCase();
      this.filter();
    },

    /**
     * Filter policies. Only policies whose name contains the filter pattern are
     * shown in the table. Furthermore, policies whose value is not currently
     * set are only shown if the corresponding checkbox is checked.
     */
    filter: function() {
      var showUnset = $('show-unset').checked;
      var policies = this.getElementsByTagName('tbody');
      for (var i = 0; i < policies.length; i++) {
        var policy = policies[i];
        policy.hidden =
            policy.unset && !showUnset ||
            policy.name.toLowerCase().indexOf(this.filterPattern_) == -1;
      }
      if (this.querySelector('tbody:not([hidden])'))
        this.parentElement.classList.remove('empty');
      else
        this.parentElement.classList.add('empty');
      setTimeout(this.checkOverflow_.bind(this), 0);
    },

    /**
     * Check the table columns for overflow.
     * @private
     */
    checkOverflow_: function() {
      var policies = this.getElementsByTagName('tbody');
      for (var i = 0; i < policies.length; i++) {
        if (!policies[i].hidden)
          policies[i].checkOverflow();
      }
    },

    /**
     * Add a policy with the given |name| and |value| to the table.
     * @param {string} name The policy name.
     * @param {Object} value Dictionary with information about the policy value.
     * @param {boolean} unknown Whether the policy name is not recoginzed.
     * @private
     */
    setPolicyValue_: function(name, value, unknown) {
      var policy = new Policy;
      var includeStatus = this.querySelector('.status-column') != null;
      policy.initialize(name, value, unknown, includeStatus);
      this.appendChild(policy);
    },
  };

  /**
   * A singelton object that handles communication between browser and WebUI.
   * @constructor
   */
  function Page() {
  }

  // Make Page a singleton.
  cr.addSingletonGetter(Page);

  /**
   * Provide a list of all known policies to the UI. Called by the browser on
   * page load.
   * @param {Object} names Dictionary containing all known policy names.
   */
  Page.setPolicyNames = function(names) {
    var table = this.getInstance().policyTables['chrome'];
    table.setPolicyNames(names);
  };

  /**
   * Provide a list of the currently set policy values and any errors detected
   * while parsing these to the UI. Called by the browser on page load and
   * whenever policy values change.
   * @param {Object} values Dictionary containing the current policy values.
   */
  Page.setPolicyValues = function(values) {
    var page = this.getInstance();

    if (values.hasOwnProperty('chromePolicies')) {
      var table = page.policyTables['chrome'];
      table.setPolicyValues(values.chromePolicies);
    }

    if (values.hasOwnProperty('extensionPolicies')) {
      for (var extensionId in values.extensionPolicies) {
        var tableName = values.extensionPolicies[extensionId].name;
        var table = page.getOrCreateTable('extension-' + extensionId, tableName,
                                          'ID: ' + extensionId, false);
        table.setPolicyValues(values.extensionPolicies[extensionId].policies);
      }
    }
  };

  /**
   * Provide the current cloud policy status to the UI. Called by the browser on
   * page load if cloud policy is present and whenever the status changes.
   * @param {Object} status Dictionary containing the current policy status.
   */
  Page.setStatus = function(status) {
    this.getInstance().setStatus(status);
  };

  /**
   * Notify the UI that a request to reload policy values has completed. Called
   * by the browser after a request to reload policy has been sent by the UI.
   */
  Page.reloadPoliciesDone = function() {
    this.getInstance().reloadPoliciesDone();
  };

  Page.prototype = {
    /**
     * Main initialization function. Called by the browser on page load.
     */
    initialize: function() {
      uber.onContentFrameLoaded();

      this.mainSection = $('main-section');
      this.policyTables = {};

      var chromeTable = this.getOrCreateTable('chrome', 'Chrome policies', '',
                                              true);

      // Place the initial focus on the filter input field.
      $('filter').focus();

      var self = this;
      $('filter').onsearch = function(event) {
        for (policyTable in self.policyTables) {
          self.policyTables[policyTable].setFilterPattern(this.value);
        }
      };
      $('reload-policies').onclick = function(event) {
        this.disabled = true;
        chrome.send('reloadPolicies');
      };

      $('show-unset').onchange = function() {
        for (policyTable in self.policyTables) {
          self.policyTables[policyTable].filter();
        }
      };

      // Notify the browser that the page has loaded, causing it to send the
      // list of all known policies, the current policy values and the cloud
      // policy status.
      chrome.send('initialized');
    },

    /**
     * Gets the existing policy table for the given id, or if none exists,
     * creates a new policy table section, adds the section to the page,
     * and returns the new table from that section.
     * @param {string} id The key for the table in policyTables.
     * @param {string} label_title Title for this policy table.
     * @param {string} label_content Description for the policy table.
     * @return {Element} Policy table associated with the given id.
     */
    getOrCreateTable: function(id, label_title, label_content, includeStatus) {
      if (!this.policyTables.hasOwnProperty(id)) {
        var newSection = this.createPolicyTableSection(id, label_title,
          label_content, includeStatus);
        this.mainSection.appendChild(newSection);
      }
      return this.policyTables[id];
    },

    /**
     * Creates a new section containing a title, description and table of
     * policies.
     * @param {string} id Used as key when storing new table in policyTables.
     * @param {string} label_title Title for this policy table.
     * @param {string} label_content Description for the policy table.
     * @param {boolean} includeStatus Whether to display a status column.
     * @return {Element} The newly created section.
     */
    createPolicyTableSection: function(id, label_title, label_content,
                                       includeStatus) {
      var section = document.createElement('section');
      section.setAttribute('class', 'policy-table-section');

      // Add title and description.
      var title = window.document.createElement('h3');
      title.textContent = label_title;
      section.appendChild(title);

      if (label_content) {
        var description = window.document.createElement('div');
        description.classList.add('table-description');
        description.textContent = label_content;
        section.appendChild(description);
      }

      // Add 'No Policies Set' element.
      var noPolicies = window.document.createElement('div');
      noPolicies.classList.add('no-policies-set');
      noPolicies.textContent = loadTimeData.getString('noPoliciesSet');
      section.appendChild(noPolicies);

      // Add table of policies.
      var newTable = this.createPolicyTable(includeStatus);
      this.policyTables[id] = newTable;
      section.appendChild(newTable);

      return section;
    },

    /**
     * Creates a new table for displaying policies.
     * @param {boolean} includeStatus Whether to include a status column.
     * @return {Element} The newly created table.
     */
    createPolicyTable: function(includeStatus) {
      var newTable = window.document.createElement('table');
      var tableHead = window.document.createElement('thead');
      var tableRow = window.document.createElement('tr');
      var tableHeadings = ['headerScope', 'headerLevel',
                           'headerName', 'headerValue'];

      for (var i = 0; i < tableHeadings.length; i++) {
        var tableHeader = window.document.createElement('th');
        tableHeader.textContent = loadTimeData.getString(tableHeadings[i]);
        tableRow.appendChild(tableHeader);
      }

      if (includeStatus) {
        var statusHeader = window.document.createElement('th');
        statusHeader.classList.add('status-column');
        statusHeader.textContent = loadTimeData.getString('headerStatus');
        tableRow.appendChild(statusHeader);
      }

      tableHead.appendChild(tableRow);
      newTable.appendChild(tableHead);
      cr.ui.decorate(newTable, PolicyTable);
      return newTable;
    },

    /**
     * Update the status section of the page to show the current cloud policy
     * status.
     * @param {Object} status Dictionary containing the current policy status.
     */
    setStatus: function(status) {
      // Remove any existing status boxes.
      var container = $('status-box-container');
      while (container.firstChild)
        container.removeChild(container.firstChild);
      // Hide the status section.
      var section = $('status-section');
      section.hidden = true;

      // Add a status box for each scope that has a cloud policy status.
      for (var scope in status) {
        var box = new StatusBox;
        box.initialize(scope, status[scope]);
        container.appendChild(box);
        // Show the status section.
        section.hidden = false;
      }
    },

    /**
     * Re-enable the reload policies button when the previous request to reload
     * policies values has completed.
     */
    reloadPoliciesDone: function() {
      $('reload-policies').disabled = false;
    },
  };

  return {
    Page: Page
  };
});

// Have the main initialization function be called when the page finishes
// loading.
document.addEventListener(
    'DOMContentLoaded',
    policy.Page.getInstance().initialize.bind(policy.Page.getInstance()));
