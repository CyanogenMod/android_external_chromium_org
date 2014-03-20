# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A bare-bones test server for testing cloud policy support.

This implements a simple cloud policy test server that can be used to test
chrome's device management service client. The policy information is read from
the file named device_management in the server's data directory. It contains
enforced and recommended policies for the device and user scope, and a list
of managed users.

The format of the file is JSON. The root dictionary contains a list under the
key "managed_users". It contains auth tokens for which the server will claim
that the user is managed. The token string "*" indicates that all users are
claimed to be managed. Other keys in the root dictionary identify request
scopes. The user-request scope is described by a dictionary that holds two
sub-dictionaries: "mandatory" and "recommended". Both these hold the policy
definitions as key/value stores, their format is identical to what the Linux
implementation reads from /etc.
The device-scope holds the policy-definition directly as key/value stores in the
protobuf-format.

Example:

{
  "google/chromeos/device" : {
    "guest_mode_enabled" : false
  },
  "google/chromeos/user" : {
    "mandatory" : {
      "HomepageLocation" : "http://www.chromium.org",
      "IncognitoEnabled" : false
    },
     "recommended" : {
      "JavascriptEnabled": false
    }
  },
  "google/chromeos/publicaccount/user@example.com" : {
    "mandatory" : {
      "HomepageLocation" : "http://www.chromium.org"
    },
     "recommended" : {
    }
  },
  "managed_users" : [
    "secret123456"
  ],
  "current_key_index": 0,
  "robot_api_auth_code": "fake_auth_code",
  "invalidation_source": 1025,
  "invalidation_name": "UENUPOL"
}

"""

import base64
import BaseHTTPServer
import cgi
import google.protobuf.text_format
import hashlib
import logging
import os
import random
import re
import sys
import time
import tlslite
import tlslite.api
import tlslite.utils
import tlslite.utils.cryptomath
import urlparse

# The name and availability of the json module varies in python versions.
try:
  import simplejson as json
except ImportError:
  try:
    import json
  except ImportError:
    json = None

import asn1der
import testserver_base

import device_management_backend_pb2 as dm
import cloud_policy_pb2 as cp
import chrome_extension_policy_pb2 as ep

# Device policy is only available on Chrome OS builds.
try:
  import chrome_device_policy_pb2 as dp
except ImportError:
  dp = None

# ASN.1 object identifier for PKCS#1/RSA.
PKCS1_RSA_OID = '\x2a\x86\x48\x86\xf7\x0d\x01\x01\x01'

# List of bad machine identifiers that trigger the |valid_serial_number_missing|
# flag to be set set in the policy fetch response.
BAD_MACHINE_IDS = [ '123490EN400015' ]

# List of machines that trigger the server to send kiosk enrollment response
# for the register request.
KIOSK_MACHINE_IDS = [ 'KIOSK' ]

# Dictionary containing base64-encoded policy signing keys plus per-domain
# signatures. Format is:
# {
#   'key': <base64-encoded PKCS8-format private key>,
#   'signatures': {
#     <domain1>: <base64-encdoded SHA256 signature for key + domain1>
#     <domain2>: <base64-encdoded SHA256 signature for key + domain2>
#     ...
#   }
# }
SIGNING_KEYS = [
    # Key1
    {'key':
       'MIIBVQIBADANBgkqhkiG9w0BAQEFAASCAT8wggE7AgEAAkEA2c3KzcPqvnJ5HCk3OZkf1'
       'LMO8Ht4dw4FO2U0EmKvpo0zznj4RwUdmKobH1AFWzwZP4CDY2M67MsukE/1Jnbx1QIDAQ'
       'ABAkBkKcLZa/75hHVz4PR3tZaw34PATlfxEG6RiRIwXlf/FFlfGIZOSxdW/I1A3XRl0/9'
       'nZMuctBSKBrcTRZQWfT/hAiEA9g8xbQbMO6BEH/XCRSsQbPlvj4c9wDtVEzeAzZ/ht9kC'
       'IQDiml+/lXS1emqml711jJcYJNYJzdy1lL/ieKogR59oXQIhAK+Pl4xa1U2VxAWpq7r+R'
       'vH55wdZT03hB4p2h4gvEzXBAiAkw9kvE0eZPiBZoRrrHIFTOH7FnnHlwBmV2+/2RsiVPQ'
       'IhAKqx/4qisivvmoM/xbzUagfoxwsu1A/4mGjhBKiS0BCq',
     'signatures':
       {'example.com':
          'l+sT5mziei/GbmiP7VtRCCfwpZcg7uKbW2OlnK5B/TTELutjEIAMdHduNBwbO44qOn'
          '/5c7YrtkXbBehaaDYFPGI6bGTbDmG9KRxhS+DaB7opgfCQWLi79Gn/jytKLZhRN/VS'
          'y+PEbezqMi3d1/xDxlThwWZDNwnhv9ER/Nu/32ZTjzgtqonSn2CQtwXCIILm4FdV/1'
          '/BdmZG+Ge4i4FTqYtInir5YFe611KXU/AveGhQGBIAXo4qYg1IqbVrvKBSU9dlI6Sl'
          '9TJJLbJ3LGaXuljgFhyMAl3gcy7ftC9MohEmwa+sc7y2mOAgYQ5SSmyAtQwQgAkX9J'
          '3+tfxjmoA/dg==',
        'chromepolicytest.com':
          'TzBiigZKwBdr6lyP6tUDsw+Q9wYO1Yepyxm0O4JZ4RID32L27sWzC1/hwC51fRcCvP'
          'luEVIW6mH+BFODXMrteUFWfbbG7jgV+Wg+QdzMqgJjxhNKFXPTsZ7/286LAd1vBY/A'
          'nGd8Wog6AhzfrgMbLNsH794GD0xIUwRvXUWFNP8pClj5VPgQnJrIA9aZwW8FNGbteA'
          'HacFB0T/oqP5s7XT4Qvkj14RLmCgTwEM8Vcpqy5teJaF8yN17wniveddoOQGH6s0HC'
          'ocprEccrH5fP/WVAPxCfx4vVYQY5q4CZ4K3f6dTC2FV4IDelM6dugEkvSS02YCzDaO'
          'N+Z7IwElzTKg==',
        'managedchrome.com':
          'T0wXC5w3GXyovA09pyOLX7ui/NI603UfbZXYyTbHI7xtzCIaHVPH35Nx4zdqVrdsej'
          'ErQ12yVLDDIJokY4Yl+/fj/zrkAPxThI+TNQ+jo0i+al05PuopfpzvCzIXiZBbkbyW'
          '3XfedxXP3IPN2XU2/3vX+ZXUNG6pxeETem64kGezkjkUraqnHw3JVzwJYHhpMcwdLP'
          'PYK6V23BbEHEVBtQZd/ledXacz7gOzm1zGni4e+vxA2roAdJWyhbjU0dTKNNUsZmMv'
          'ryQH9Af1Jw+dqs0RAbhcJXm2i8EUWIgNv6aMn1Z2DzZwKKjXsKgcYSRo8pdYa8RZAo'
          'UExd9roA9a5w==',
        }
     },
    # Key2
    {'key':
       'MIIBVAIBADANBgkqhkiG9w0BAQEFAASCAT4wggE6AgEAAkEAmZhreV04M3knCi6wibr49'
       'oDesHny1G33PKOX9ko8pcxAiu9ZqsKCj7wNW2PGqnLi81fddACwQtYn5xdhCtzB9wIDAQ'
       'ABAkA0z8m0cy8N08xundspoFZWO71WJLgv/peSDBYGI0RzJR1l9Np355EukQUQwRs5XrL'
       '3vRQZy2vDqeiR96epkAhRAiEAzJ4DVI8k3pAl7CGv5icqFkJ02viExIwehhIEXBcB6p0C'
       'IQDAKmzpoRpBEZRQ9xrTvPOi+Ea8Jnd478BU7CI/LFfgowIgMfLIoVWoDGRnvXKju60Hy'
       'xNB70oHLut9cADp64j6QMkCIDrgxN4QbmrhaAAmtiGKE1wrlgCwCIsVamiasSOKAqLhAi'
       'EAo/ItVcFtQPod97qG71CY/O4JzOciuU6AMhprs181vfM=',
     'signatures':
       # Key2 signatures
       {'example.com':
          'cO0nQjRptkeefKDw5QpJSQDavHABxUvbR9Wvoa235OG9Whw1RFqq2ye6pKnI3ezW6/'
          '7b4ANcpi5a7HV5uF8K7gWyYdxY8NHLeyrbwXxg5j6HAmHmkP1UZcf/dAnWqo7cW8g4'
          'DIQOhC43KkveMYJ2HnelwdXt/7zqkbe8/3Yj4nhjAUeARx86Sb8Nzydwkrvqs5Jw/x'
          '5LG+BODExrXXcGu/ubDlW4ivJFqfNUPQysqBXSMY2XCHPJDx3eECLGVVN/fFAWWgjM'
          'HFObAriAt0b18cc9Nr0mAt4Qq1oDzWcAHCPHE+5dr8Uf46BUrMLJRNRKCY7rrsoIin'
          '9Be9gs3W+Aww==',
        'chromepolicytest.com':
          'mr+9CCYvR0cTvPwlzkxqlpGYy55gY7cPiIkPAPoql51yHK1tkMTOSFru8Dy/nMt+0o'
          '4z7WO60F1wnIBGkQxnTj/DsO6QpCYi7oHqtLmZ2jsLQFlMyvPGUtpJEFvRwjr/TNbh'
          '6RqUtz1LQFuJQ848kBrx7nkte1L8SuPDExgx+Q3LtbNj4SuTdvMUBMvEERXiLuwfFL'
          'BefGjtsqfWETQVlJTCW7xcqOLedIX8UYgEDBpDOZ23A3GzCShuBsIut5m87R5mODht'
          'EUmKNDK1+OMc6SyDpf+r48Wph4Db1bVaKy8fcpSNJOwEgsrmH7/+owKPGcN7I5jYAF'
          'Z2PGxHTQ9JNA==',
        'managedchrome.com':
          'o5MVSo4bRwIJ/aooGyXpRXsEsWPG8fNA2UTG8hgwnLYhNeJCCnLs/vW2vdp0URE8jn'
          'qiG4N8KjbuiGw0rJtO1EygdLfpnMEtqYlFjrOie38sy92l/AwohXj6luYzMWL+FqDu'
          'WQeXasjgyY4s9BOLQVDEnEj3pvqhrk/mXvMwUeXGpbxTNbWAd0C8BTZrGOwU/kIXxo'
          'vAMGg8L+rQaDwBTEnMsMZcvlrIyqSg5v4BxCWuL3Yd2xvUqZEUWRp1aKetsHRnz5hw'
          'H7WK7DzvKepDn06XjPG9lchi448U3HB3PRKtCzfO3nD9YXMKTuqRpKPF8PeK11CWh1'
          'DBvBYwi20vbQ==',
       },
    },
]

class PolicyRequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):
  """Decodes and handles device management requests from clients.

  The handler implements all the request parsing and protobuf message decoding
  and encoding. It calls back into the server to lookup, register, and
  unregister clients.
  """

  def __init__(self, request, client_address, server):
    """Initialize the handler.

    Args:
      request: The request data received from the client as a string.
      client_address: The client address.
      server: The TestServer object to use for (un)registering clients.
    """
    BaseHTTPServer.BaseHTTPRequestHandler.__init__(self, request,
                                                   client_address, server)

  def GetUniqueParam(self, name):
    """Extracts a unique query parameter from the request.

    Args:
      name: Names the parameter to fetch.
    Returns:
      The parameter value or None if the parameter doesn't exist or is not
      unique.
    """
    if not hasattr(self, '_params'):
      self._params = cgi.parse_qs(self.path[self.path.find('?') + 1:])

    param_list = self._params.get(name, [])
    if len(param_list) == 1:
      return param_list[0]
    return None

  def do_GET(self):
    """Handles GET requests.

    Currently this is only used to serve external policy data."""
    sep = self.path.find('?')
    path = self.path if sep == -1 else self.path[:sep]
    if path == '/externalpolicydata':
      http_response, raw_reply = self.HandleExternalPolicyDataRequest()
    else:
      http_response = 404
      raw_reply = 'Invalid path'
    self.send_response(http_response)
    self.end_headers()
    self.wfile.write(raw_reply)

  def do_POST(self):
    http_response, raw_reply = self.HandleRequest()
    self.send_response(http_response)
    if (http_response == 200):
      self.send_header('Content-Type', 'application/x-protobuffer')
    self.end_headers()
    self.wfile.write(raw_reply)

  def HandleExternalPolicyDataRequest(self):
    """Handles a request to download policy data for a component."""
    policy_key = self.GetUniqueParam('key')
    if not policy_key:
      return (400, 'Missing key parameter')
    data = self.server.ReadPolicyDataFromDataDir(policy_key)
    if data is None:
      return (404, 'Policy not found for ' + policy_key)
    return (200, data)

  def HandleRequest(self):
    """Handles a request.

    Parses the data supplied at construction time and returns a pair indicating
    http status code and response data to be sent back to the client.

    Returns:
      A tuple of HTTP status code and response data to send to the client.
    """
    rmsg = dm.DeviceManagementRequest()
    length = int(self.headers.getheader('content-length'))
    rmsg.ParseFromString(self.rfile.read(length))

    logging.debug('gaia auth token -> ' +
                  self.headers.getheader('Authorization', ''))
    logging.debug('oauth token -> ' + str(self.GetUniqueParam('oauth_token')))
    logging.debug('deviceid -> ' + str(self.GetUniqueParam('deviceid')))
    self.DumpMessage('Request', rmsg)

    request_type = self.GetUniqueParam('request')
    # Check server side requirements, as defined in
    # device_management_backend.proto.
    if (self.GetUniqueParam('devicetype') != '2' or
        self.GetUniqueParam('apptype') != 'Chrome' or
        (request_type != 'ping' and
         len(self.GetUniqueParam('deviceid')) >= 64) or
        len(self.GetUniqueParam('agent')) >= 64):
      return (400, 'Invalid request parameter')
    if request_type == 'register':
      response = self.ProcessRegister(rmsg.register_request)
    elif request_type == 'api_authorization':
      response = self.ProcessApiAuthorization(rmsg.service_api_access_request)
    elif request_type == 'unregister':
      response = self.ProcessUnregister(rmsg.unregister_request)
    elif request_type == 'policy' or request_type == 'ping':
      response = self.ProcessPolicy(rmsg, request_type)
    elif request_type == 'enterprise_check':
      response = self.ProcessAutoEnrollment(rmsg.auto_enrollment_request)
    elif request_type == 'device_state_retrieval':
      response = self.ProcessDeviceStateRetrievalRequest(
          rmsg.device_state_retrieval_request)
    else:
      return (400, 'Invalid request parameter')

    self.DumpMessage('Response', response[1])
    return (response[0], response[1].SerializeToString())

  def CreatePolicyForExternalPolicyData(self, policy_key):
    """Returns an ExternalPolicyData protobuf for policy_key.

    If there is policy data for policy_key then the download url will be
    set so that it points to that data, and the appropriate hash is also set.
    Otherwise, the protobuf will be empty.

    Args:
      policy_key: the policy type and settings entity id, joined by '/'.

    Returns:
      A serialized ExternalPolicyData.
    """
    settings = ep.ExternalPolicyData()
    data = self.server.ReadPolicyDataFromDataDir(policy_key)
    if data:
      settings.download_url = urlparse.urljoin(
          self.server.GetBaseURL(), 'externalpolicydata?key=%s' % policy_key)
      settings.secure_hash = hashlib.sha256(data).digest()
    return settings.SerializeToString()

  def CheckGoogleLogin(self):
    """Extracts the auth token from the request and returns it. The token may
    either be a GoogleLogin token from an Authorization header, or an OAuth V2
    token from the oauth_token query parameter. Returns None if no token is
    present.
    """
    oauth_token = self.GetUniqueParam('oauth_token')
    if oauth_token:
      return oauth_token

    match = re.match('GoogleLogin auth=(\\w+)',
                     self.headers.getheader('Authorization', ''))
    if match:
      return match.group(1)

    return None

  def ProcessRegister(self, msg):
    """Handles a register request.

    Checks the query for authorization and device identifier, registers the
    device with the server and constructs a response.

    Args:
      msg: The DeviceRegisterRequest message received from the client.

    Returns:
      A tuple of HTTP status code and response data to send to the client.
    """
    # Check the auth token and device ID.
    auth = self.CheckGoogleLogin()
    if not auth:
      return (403, 'No authorization')

    policy = self.server.GetPolicies()
    if ('*' not in policy['managed_users'] and
        auth not in policy['managed_users']):
      return (403, 'Unmanaged')

    device_id = self.GetUniqueParam('deviceid')
    if not device_id:
      return (400, 'Missing device identifier')

    token_info = self.server.RegisterDevice(device_id,
                                             msg.machine_id,
                                             msg.type)

    # Send back the reply.
    response = dm.DeviceManagementResponse()
    response.register_response.device_management_token = (
        token_info['device_token'])
    response.register_response.machine_name = token_info['machine_name']
    response.register_response.enrollment_type = token_info['enrollment_mode']

    return (200, response)

  def ProcessApiAuthorization(self, msg):
    """Handles an API authorization request.

    Args:
      msg: The DeviceServiceApiAccessRequest message received from the client.

    Returns:
      A tuple of HTTP status code and response data to send to the client.
    """
    policy = self.server.GetPolicies()

    # Return the auth code from the config file if it's defined,
    # else return a descriptive default value.
    response = dm.DeviceManagementResponse()
    response.service_api_access_response.auth_code = policy.get(
        'robot_api_auth_code', 'policy_testserver.py-auth_code')

    return (200, response)

  def ProcessUnregister(self, msg):
    """Handles a register request.

    Checks for authorization, unregisters the device and constructs the
    response.

    Args:
      msg: The DeviceUnregisterRequest message received from the client.

    Returns:
      A tuple of HTTP status code and response data to send to the client.
    """
    # Check the management token.
    token, response = self.CheckToken()
    if not token:
      return response

    # Unregister the device.
    self.server.UnregisterDevice(token['device_token'])

    # Prepare and send the response.
    response = dm.DeviceManagementResponse()
    response.unregister_response.CopyFrom(dm.DeviceUnregisterResponse())

    return (200, response)

  def ProcessPolicy(self, msg, request_type):
    """Handles a policy request.

    Checks for authorization, encodes the policy into protobuf representation
    and constructs the response.

    Args:
      msg: The DeviceManagementRequest message received from the client.

    Returns:
      A tuple of HTTP status code and response data to send to the client.
    """
    token_info, error = self.CheckToken()
    if not token_info:
      return error

    key_update_request = msg.device_state_key_update_request
    if len(key_update_request.server_backed_state_key) > 0:
      self.server.UpdateStateKeys(token_info['device_token'],
                                  key_update_request.server_backed_state_key)

    response = dm.DeviceManagementResponse()
    for request in msg.policy_request.request:
      fetch_response = response.policy_response.response.add()
      if (request.policy_type in
             ('google/android/user',
              'google/chrome/extension',
              'google/chromeos/device',
              'google/chromeos/publicaccount',
              'google/chromeos/user',
              'google/chrome/user',
              'google/ios/user')):
        if request_type != 'policy':
          fetch_response.error_code = 400
          fetch_response.error_message = 'Invalid request type'
        else:
          self.ProcessCloudPolicy(request, token_info, fetch_response)
      else:
        fetch_response.error_code = 400
        fetch_response.error_message = 'Invalid policy_type'

    return (200, response)

  def ProcessAutoEnrollment(self, msg):
    """Handles an auto-enrollment check request.

    The reply depends on the value of the modulus:
      1: replies with no new modulus and the sha256 hash of "0"
      2: replies with a new modulus, 4.
      4: replies with a new modulus, 2.
      8: fails with error 400.
      16: replies with a new modulus, 16.
      32: replies with a new modulus, 1.
      anything else: replies with no new modulus and an empty list of hashes

    These allow the client to pick the testing scenario its wants to simulate.

    Args:
      msg: The DeviceAutoEnrollmentRequest message received from the client.

    Returns:
      A tuple of HTTP status code and response data to send to the client.
    """
    auto_enrollment_response = dm.DeviceAutoEnrollmentResponse()

    if msg.modulus == 1:
      auto_enrollment_response.hash.extend(
          self.server.GetMatchingStateKeyHashes(msg.modulus, msg.remainder))
    elif msg.modulus == 2:
      auto_enrollment_response.expected_modulus = 4
    elif msg.modulus == 4:
      auto_enrollment_response.expected_modulus = 2
    elif msg.modulus == 8:
      return (400, 'Server error')
    elif msg.modulus == 16:
      auto_enrollment_response.expected_modulus = 16
    elif msg.modulus == 32:
      auto_enrollment_response.expected_modulus = 1

    response = dm.DeviceManagementResponse()
    response.auto_enrollment_response.CopyFrom(auto_enrollment_response)
    return (200, response)

  def ProcessDeviceStateRetrievalRequest(self, msg):
    """Handles a device state retrieval request.

    Response data is taken from server configuration.

    Returns:
      A tuple of HTTP status code and response data to send to the client.
    """
    device_state_retrieval_response = dm.DeviceStateRetrievalResponse()

    client = self.server.LookupByStateKey(msg.server_backed_state_key)
    if client is not None:
      state = self.server.GetPolicies().get('device_state', {})
      FIELDS = [
          'management_domain',
          'restore_mode',
      ]
      for field in FIELDS:
        if field in state:
          setattr(device_state_retrieval_response, field, state[field])

    response = dm.DeviceManagementResponse()
    response.device_state_retrieval_response.CopyFrom(
        device_state_retrieval_response)
    return (200, response)

  def SetProtobufMessageField(self, group_message, field, field_value):
    '''Sets a field in a protobuf message.

    Args:
      group_message: The protobuf message.
      field: The field of the message to set, it should be a member of
          group_message.DESCRIPTOR.fields.
      field_value: The value to set.
    '''
    if field.label == field.LABEL_REPEATED:
      assert type(field_value) == list
      entries = group_message.__getattribute__(field.name)
      if field.message_type is None:
        for list_item in field_value:
          entries.append(list_item)
      else:
        # This field is itself a protobuf.
        sub_type = field.message_type
        for sub_value in field_value:
          assert type(sub_value) == dict
          # Add a new sub-protobuf per list entry.
          sub_message = entries.add()
          # Now iterate over its fields and recursively add them.
          for sub_field in sub_message.DESCRIPTOR.fields:
            if sub_field.name in sub_value:
              value = sub_value[sub_field.name]
              self.SetProtobufMessageField(sub_message, sub_field, value)
      return
    elif field.type == field.TYPE_BOOL:
      assert type(field_value) == bool
    elif field.type == field.TYPE_STRING:
      assert type(field_value) == str or type(field_value) == unicode
    elif field.type == field.TYPE_INT64:
      assert type(field_value) == int
    elif (field.type == field.TYPE_MESSAGE and
          field.message_type.name == 'StringList'):
      assert type(field_value) == list
      entries = group_message.__getattribute__(field.name).entries
      for list_item in field_value:
        entries.append(list_item)
      return
    else:
      raise Exception('Unknown field type %s' % field.type)
    group_message.__setattr__(field.name, field_value)

  def GatherDevicePolicySettings(self, settings, policies):
    '''Copies all the policies from a dictionary into a protobuf of type
    CloudDeviceSettingsProto.

    Args:
      settings: The destination ChromeDeviceSettingsProto protobuf.
      policies: The source dictionary containing policies in JSON format.
    '''
    for group in settings.DESCRIPTOR.fields:
      # Create protobuf message for group.
      group_message = eval('dp.' + group.message_type.name + '()')
      # Indicates if at least one field was set in |group_message|.
      got_fields = False
      # Iterate over fields of the message and feed them from the
      # policy config file.
      for field in group_message.DESCRIPTOR.fields:
        field_value = None
        if field.name in policies:
          got_fields = True
          field_value = policies[field.name]
          self.SetProtobufMessageField(group_message, field, field_value)
      if got_fields:
        settings.__getattribute__(group.name).CopyFrom(group_message)

  def GatherUserPolicySettings(self, settings, policies):
    '''Copies all the policies from a dictionary into a protobuf of type
    CloudPolicySettings.

    Args:
      settings: The destination: a CloudPolicySettings protobuf.
      policies: The source: a dictionary containing policies under keys
          'recommended' and 'mandatory'.
    '''
    for field in settings.DESCRIPTOR.fields:
      # |field| is the entry for a specific policy in the top-level
      # CloudPolicySettings proto.

      # Look for this policy's value in the mandatory or recommended dicts.
      if field.name in policies.get('mandatory', {}):
        mode = cp.PolicyOptions.MANDATORY
        value = policies['mandatory'][field.name]
      elif field.name in policies.get('recommended', {}):
        mode = cp.PolicyOptions.RECOMMENDED
        value = policies['recommended'][field.name]
      else:
        continue

      # Create protobuf message for this policy.
      policy_message = eval('cp.' + field.message_type.name + '()')
      policy_message.policy_options.mode = mode
      field_descriptor = policy_message.DESCRIPTOR.fields_by_name['value']
      self.SetProtobufMessageField(policy_message, field_descriptor, value)
      settings.__getattribute__(field.name).CopyFrom(policy_message)

  def ProcessCloudPolicy(self, msg, token_info, response):
    """Handles a cloud policy request. (New protocol for policy requests.)

    Encodes the policy into protobuf representation, signs it and constructs
    the response.

    Args:
      msg: The CloudPolicyRequest message received from the client.
      token_info: the token extracted from the request.
      response: A PolicyFetchResponse message that should be filled with the
                response data.
    """

    if msg.machine_id:
      self.server.UpdateMachineId(token_info['device_token'], msg.machine_id)

    # Response is only given if the scope is specified in the config file.
    # Normally 'google/chromeos/device', 'google/chromeos/user' and
    # 'google/chromeos/publicaccount' should be accepted.
    policy = self.server.GetPolicies()
    policy_value = ''
    policy_key = msg.policy_type
    if msg.settings_entity_id:
      policy_key += '/' + msg.settings_entity_id
    if msg.policy_type in token_info['allowed_policy_types']:
      if msg.policy_type in ('google/android/user',
                             'google/chromeos/publicaccount',
                             'google/chromeos/user',
                             'google/chrome/user',
                             'google/ios/user'):
        settings = cp.CloudPolicySettings()
        payload = self.server.ReadPolicyFromDataDir(policy_key, settings)
        if payload is None:
          self.GatherUserPolicySettings(settings, policy.get(policy_key, {}))
          payload = settings.SerializeToString()
      elif dp is not None and msg.policy_type == 'google/chromeos/device':
        settings = dp.ChromeDeviceSettingsProto()
        payload = self.server.ReadPolicyFromDataDir(policy_key, settings)
        if payload is None:
          self.GatherDevicePolicySettings(settings, policy.get(policy_key, {}))
          payload = settings.SerializeToString()
      elif msg.policy_type == 'google/chrome/extension':
        settings = ep.ExternalPolicyData()
        payload = self.server.ReadPolicyFromDataDir(policy_key, settings)
        if payload is None:
          payload = self.CreatePolicyForExternalPolicyData(policy_key)
      else:
        response.error_code = 400
        response.error_message = 'Invalid policy type'
        return
    else:
      response.error_code = 400
      response.error_message = 'Request not allowed for the token used'
      return

    # Sign with 'current_key_index', defaulting to key 0.
    signing_key = None
    req_key = None
    current_key_index = policy.get('current_key_index', 0)
    nkeys = len(self.server.keys)
    if (msg.signature_type == dm.PolicyFetchRequest.SHA1_RSA and
        current_key_index in range(nkeys)):
      signing_key = self.server.keys[current_key_index]
      if msg.public_key_version in range(1, nkeys + 1):
        # requested key exists, use for signing and rotate.
        req_key = self.server.keys[msg.public_key_version - 1]['private_key']

    # Fill the policy data protobuf.
    policy_data = dm.PolicyData()
    policy_data.policy_type = msg.policy_type
    policy_data.timestamp = int(time.time() * 1000)
    policy_data.request_token = token_info['device_token']
    policy_data.policy_value = payload
    policy_data.machine_name = token_info['machine_name']
    policy_data.valid_serial_number_missing = (
        token_info['machine_id'] in BAD_MACHINE_IDS)
    policy_data.settings_entity_id = msg.settings_entity_id
    policy_data.service_account_identity = policy.get(
        'service_account_identity',
        'policy_testserver.py-service_account_identity')
    invalidation_source = policy.get('invalidation_source')
    if invalidation_source is not None:
      policy_data.invalidation_source = invalidation_source
    # Since invalidation_name is type bytes in the proto, the Unicode name
    # provided needs to be encoded as ASCII to set the correct byte pattern.
    invalidation_name = policy.get('invalidation_name')
    if invalidation_name is not None:
      policy_data.invalidation_name = invalidation_name.encode('ascii')

    if signing_key:
      policy_data.public_key_version = current_key_index + 1
    if msg.policy_type == 'google/chromeos/publicaccount':
      policy_data.username = msg.settings_entity_id
    else:
      # For regular user/device policy, there is no way for the testserver to
      # know the user name belonging to the GAIA auth token we received (short
      # of actually talking to GAIA). To address this, we read the username from
      # the policy configuration dictionary, or use a default.
      policy_data.username = policy.get('policy_user', 'user@example.com')
    policy_data.device_id = token_info['device_id']
    signed_data = policy_data.SerializeToString()

    response.policy_data = signed_data
    if signing_key:
      response.policy_data_signature = (
          signing_key['private_key'].hashAndSign(signed_data).tostring())
      if msg.public_key_version != current_key_index + 1:
        response.new_public_key = signing_key['public_key']

        # Set the verification signature appropriate for the policy domain.
        # TODO(atwilson): Use the enrollment domain for public accounts when
        # we add key validation for ChromeOS (http://crbug.com/328038).
        if 'signatures' in signing_key:
          verification_sig = self.GetSignatureForDomain(
              signing_key['signatures'], policy_data.username)

          if verification_sig:
            assert len(verification_sig) == 256, \
                'bad signature size: %d' % len(verification_sig)
            response.new_public_key_verification_signature = verification_sig

        if req_key:
          response.new_public_key_signature = (
              req_key.hashAndSign(response.new_public_key).tostring())

    return (200, response.SerializeToString())

  def GetSignatureForDomain(self, signatures, username):
    parsed_username = username.split("@", 1)
    if len(parsed_username) != 2:
      logging.error('Could not extract domain from username: %s' % username)
      return None
    domain = parsed_username[1]

    # Lookup the domain's signature in the passed dictionary. If none is found,
    # fallback to a wildcard signature.
    if domain in signatures:
      return signatures[domain]
    if '*' in signatures:
      return signatures['*']

    # No key matching this domain.
    logging.error('No verification signature matching domain: %s' % domain)
    return None

  def CheckToken(self):
    """Helper for checking whether the client supplied a valid DM token.

    Extracts the token from the request and passed to the server in order to
    look up the client.

    Returns:
      A pair of token information record and error response. If the first
      element is None, then the second contains an error code to send back to
      the client. Otherwise the first element is the same structure that is
      returned by LookupToken().
    """
    error = 500
    dmtoken = None
    request_device_id = self.GetUniqueParam('deviceid')
    match = re.match('GoogleDMToken token=(\\w+)',
                     self.headers.getheader('Authorization', ''))
    if match:
      dmtoken = match.group(1)
    if not dmtoken:
      error = 401
    else:
      token_info = self.server.LookupToken(dmtoken)
      if (not token_info or
          not request_device_id or
          token_info['device_id'] != request_device_id):
        error = 410
      else:
        return (token_info, None)

    logging.debug('Token check failed with error %d' % error)

    return (None, (error, 'Server error %d' % error))

  def DumpMessage(self, label, msg):
    """Helper for logging an ASCII dump of a protobuf message."""
    logging.debug('%s\n%s' % (label, str(msg)))


class PolicyTestServer(testserver_base.BrokenPipeHandlerMixIn,
                       testserver_base.StoppableHTTPServer):
  """Handles requests and keeps global service state."""

  def __init__(self, server_address, data_dir, policy_path, client_state_file,
               private_key_paths, server_base_url):
    """Initializes the server.

    Args:
      server_address: Server host and port.
      policy_path: Names the file to read JSON-formatted policy from.
      private_key_paths: List of paths to read private keys from.
    """
    testserver_base.StoppableHTTPServer.__init__(self, server_address,
                                                 PolicyRequestHandler)
    self._registered_tokens = {}
    self.data_dir = data_dir
    self.policy_path = policy_path
    self.client_state_file = client_state_file
    self.server_base_url = server_base_url

    self.keys = []
    if private_key_paths:
      # Load specified keys from the filesystem.
      for key_path in private_key_paths:
        try:
          key_str = open(key_path).read()
        except IOError:
          print 'Failed to load private key from %s' % key_path
          continue
        try:
          key = tlslite.api.parsePEMKey(key_str, private=True)
        except SyntaxError:
          key = tlslite.utils.Python_RSAKey.Python_RSAKey._parsePKCS8(
              tlslite.utils.cryptomath.stringToBytes(key_str))

        assert key is not None
        key_info = { 'private_key' : key }

        # Now try to read in a signature, if one exists.
        try:
          key_sig = open(key_path + '.sig').read()
          # Create a dictionary with the wildcard domain + signature
          key_info['signatures'] = {'*': key_sig}
        except IOError:
          print 'Failed to read validation signature from %s.sig' % key_path
        self.keys.append(key_info)
    else:
      # Use the canned private keys if none were passed from the command line.
      for signing_key in SIGNING_KEYS:
        decoded_key = base64.b64decode(signing_key['key']);
        key = tlslite.utils.Python_RSAKey.Python_RSAKey._parsePKCS8(
            tlslite.utils.cryptomath.stringToBytes(decoded_key))
        assert key is not None
        # Grab the signature dictionary for this key and decode all of the
        # signatures.
        signature_dict = signing_key['signatures']
        decoded_signatures = {}
        for domain in signature_dict:
          decoded_signatures[domain] = base64.b64decode(signature_dict[domain])
        self.keys.append({'private_key': key,
                          'signatures': decoded_signatures})

    # Derive the public keys from the private keys.
    for entry in self.keys:
      key = entry['private_key']

      algorithm = asn1der.Sequence(
          [ asn1der.Data(asn1der.OBJECT_IDENTIFIER, PKCS1_RSA_OID),
            asn1der.Data(asn1der.NULL, '') ])
      rsa_pubkey = asn1der.Sequence([ asn1der.Integer(key.n),
                                      asn1der.Integer(key.e) ])
      pubkey = asn1der.Sequence([ algorithm, asn1der.Bitstring(rsa_pubkey) ])
      entry['public_key'] = pubkey

    # Load client state.
    if self.client_state_file is not None:
      try:
        file_contents = open(self.client_state_file).read()
        self._registered_tokens = json.loads(file_contents, strict=False)
      except IOError:
        pass

  def GetPolicies(self):
    """Returns the policies to be used, reloaded form the backend file every
       time this is called.
    """
    policy = {}
    if json is None:
      print 'No JSON module, cannot parse policy information'
    else :
      try:
        policy = json.loads(open(self.policy_path).read(), strict=False)
      except IOError:
        print 'Failed to load policy from %s' % self.policy_path
    return policy

  def RegisterDevice(self, device_id, machine_id, type):
    """Registers a device or user and generates a DM token for it.

    Args:
      device_id: The device identifier provided by the client.

    Returns:
      The newly generated device token for the device.
    """
    dmtoken_chars = []
    while len(dmtoken_chars) < 32:
      dmtoken_chars.append(random.choice('0123456789abcdef'))
    dmtoken = ''.join(dmtoken_chars)
    allowed_policy_types = {
      dm.DeviceRegisterRequest.BROWSER: [
          'google/chrome/user',
          'google/chrome/extension'
      ],
      dm.DeviceRegisterRequest.USER: [
          'google/chromeos/user',
          'google/chrome/extension'
      ],
      dm.DeviceRegisterRequest.DEVICE: [
          'google/chromeos/device',
          'google/chromeos/publicaccount'
      ],
      dm.DeviceRegisterRequest.ANDROID_BROWSER: [
          'google/android/user'
      ],
      dm.DeviceRegisterRequest.IOS_BROWSER: [
          'google/ios/user'
      ],
      dm.DeviceRegisterRequest.TT: ['google/chromeos/user',
                                    'google/chrome/user'],
    }
    if machine_id in KIOSK_MACHINE_IDS:
      enrollment_mode = dm.DeviceRegisterResponse.RETAIL
    else:
      enrollment_mode = dm.DeviceRegisterResponse.ENTERPRISE
    self._registered_tokens[dmtoken] = {
      'device_id': device_id,
      'device_token': dmtoken,
      'allowed_policy_types': allowed_policy_types[type],
      'machine_name': 'chromeos-' + machine_id,
      'machine_id': machine_id,
      'enrollment_mode': enrollment_mode,
    }
    self.WriteClientState()
    return self._registered_tokens[dmtoken]

  def UpdateMachineId(self, dmtoken, machine_id):
    """Updates the machine identifier for a registered device.

    Args:
      dmtoken: The device management token provided by the client.
      machine_id: Updated hardware identifier value.
    """
    if dmtoken in self._registered_tokens:
      self._registered_tokens[dmtoken]['machine_id'] = machine_id
      self.WriteClientState()

  def UpdateStateKeys(self, dmtoken, state_keys):
    """Updates the state keys for a given client.

    Args:
      dmtoken: The device management token provided by the client.
      state_keys: The state keys to set.
    """
    if dmtoken in self._registered_tokens:
      self._registered_tokens[dmtoken]['state_keys'] = map(
          lambda key : key.encode('hex'), state_keys)
      self.WriteClientState()

  def LookupToken(self, dmtoken):
    """Looks up a device or a user by DM token.

    Args:
      dmtoken: The device management token provided by the client.

    Returns:
      A dictionary with information about a device or user that is registered by
      dmtoken, or None if the token is not found.
    """
    return self._registered_tokens.get(dmtoken, None)

  def LookupByStateKey(self, state_key):
    """Looks up a device or a user by a state key.

    Args:
      state_key: The state key provided by the client.

    Returns:
      A dictionary with information about a device or user or None if there is
      no matching record.
    """
    for client in self._registered_tokens.values():
      if state_key.encode('hex') in client.get('state_keys', []):
        return client

    return None

  def GetMatchingStateKeyHashes(self, modulus, remainder):
    """Returns all clients registered with the server.

    Returns:
      The list of registered clients.
    """
    state_keys = sum([ c.get('state_keys', [])
                       for c in self._registered_tokens.values() ], [])
    hashed_keys = map(lambda key: hashlib.sha256(key.decode('hex')).digest(),
                      set(state_keys))
    return filter(
        lambda hash : int(hash.encode('hex'), 16) & modulus == remainder,
        hashed_keys)

  def UnregisterDevice(self, dmtoken):
    """Unregisters a device identified by the given DM token.

    Args:
      dmtoken: The device management token provided by the client.
    """
    if dmtoken in self._registered_tokens.keys():
      del self._registered_tokens[dmtoken]
      self.WriteClientState()

  def WriteClientState(self):
    """Writes the client state back to the file."""
    if self.client_state_file is not None:
      json_data = json.dumps(self._registered_tokens)
      open(self.client_state_file, 'w').write(json_data)

  def GetBaseFilename(self, policy_selector):
    """Returns the base filename for the given policy_selector.

    Args:
      policy_selector: the policy type and settings entity id, joined by '/'.

    Returns:
      The filename corresponding to the policy_selector, without a file
      extension.
    """
    sanitized_policy_selector = re.sub('[^A-Za-z0-9.@-]', '_', policy_selector)
    return os.path.join(self.data_dir or '',
                        'policy_%s' % sanitized_policy_selector)

  def ReadPolicyFromDataDir(self, policy_selector, proto_message):
    """Tries to read policy payload from a file in the data directory.

    First checks for a binary rendition of the policy protobuf in
    <data_dir>/policy_<sanitized_policy_selector>.bin. If that exists, returns
    it. If that file doesn't exist, tries
    <data_dir>/policy_<sanitized_policy_selector>.txt and decodes that as a
    protobuf using proto_message. If that fails as well, returns None.

    Args:
      policy_selector: Selects which policy to read.
      proto_message: Optional protobuf message object used for decoding the
          proto text format.

    Returns:
      The binary payload message, or None if not found.
    """
    base_filename = self.GetBaseFilename(policy_selector)

    # Try the binary payload file first.
    try:
      return open(base_filename + '.bin').read()
    except IOError:
      pass

    # If that fails, try the text version instead.
    if proto_message is None:
      return None

    try:
      text = open(base_filename + '.txt').read()
      google.protobuf.text_format.Merge(text, proto_message)
      return proto_message.SerializeToString()
    except IOError:
      return None
    except google.protobuf.text_format.ParseError:
      return None

  def ReadPolicyDataFromDataDir(self, policy_selector):
    """Returns the external policy data for |policy_selector| if found.

    Args:
      policy_selector: Selects which policy to read.

    Returns:
      The data for the corresponding policy type and entity id, if found.
    """
    base_filename = self.GetBaseFilename(policy_selector)
    try:
      return open(base_filename + '.data').read()
    except IOError:
      return None

  def GetBaseURL(self):
    """Returns the server base URL.

    Respects the |server_base_url| configuration parameter, if present. Falls
    back to construct the URL from the server hostname and port otherwise.

    Returns:
      The URL to use for constructing URLs that get returned to clients.
    """
    base_url = self.server_base_url
    if base_url is None:
      base_url = 'http://%s:%s' % self.server_address[:2]

    return base_url


class PolicyServerRunner(testserver_base.TestServerRunner):

  def __init__(self):
    super(PolicyServerRunner, self).__init__()

  def create_server(self, server_data):
    data_dir = self.options.data_dir or ''
    config_file = (self.options.config_file or
                   os.path.join(data_dir, 'device_management'))
    server = PolicyTestServer((self.options.host, self.options.port),
                              data_dir, config_file,
                              self.options.client_state_file,
                              self.options.policy_keys,
                              self.options.server_base_url)
    server_data['port'] = server.server_port
    return server

  def add_options(self):
    testserver_base.TestServerRunner.add_options(self)
    self.option_parser.add_option('--client-state', dest='client_state_file',
                                  help='File that client state should be '
                                  'persisted to. This allows the server to be '
                                  'seeded by a list of pre-registered clients '
                                  'and restarts without abandoning registered '
                                  'clients.')
    self.option_parser.add_option('--policy-key', action='append',
                                  dest='policy_keys',
                                  help='Specify a path to a PEM-encoded '
                                  'private key to use for policy signing. May '
                                  'be specified multiple times in order to '
                                  'load multiple keys into the server. If the '
                                  'server has multiple keys, it will rotate '
                                  'through them in at each request in a '
                                  'round-robin fashion. The server will '
                                  'use a canned key if none is specified '
                                  'on the command line. The test server will '
                                  'also look for a verification signature file '
                                  'in the same location: <filename>.sig and if '
                                  'present will add the signature to the '
                                  'policy blob as appropriate via the '
                                  'new_public_key_verification_signature '
                                  'field.')
    self.option_parser.add_option('--log-level', dest='log_level',
                                  default='WARN',
                                  help='Log level threshold to use.')
    self.option_parser.add_option('--config-file', dest='config_file',
                                  help='Specify a configuration file to use '
                                  'instead of the default '
                                  '<data_dir>/device_management')
    self.option_parser.add_option('--server-base-url', dest='server_base_url',
                                  help='The server base URL to use when '
                                  'constructing URLs to return to the client.')

  def run_server(self):
    logger = logging.getLogger()
    logger.setLevel(getattr(logging, str(self.options.log_level).upper()))
    if (self.options.log_to_console):
      logger.addHandler(logging.StreamHandler())
    if (self.options.log_file):
      logger.addHandler(logging.FileHandler(self.options.log_file))

    testserver_base.TestServerRunner.run_server(self)


if __name__ == '__main__':
  sys.exit(PolicyServerRunner().main())
