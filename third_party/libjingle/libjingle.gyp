# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'includes': [
    '../../build/win_precompile.gypi',
  ],
  'target_defaults': {
    'defines': [
      'FEATURE_ENABLE_SSL',
      'FEATURE_ENABLE_VOICEMAIL',  # TODO(ncarter): Do we really need this?
      '_USE_32BIT_TIME_T',
      'LOGGING_INSIDE_LIBJINGLE',
      'EXPAT_RELATIVE_PATH',
      'JSONCPP_RELATIVE_PATH',
      'WEBRTC_RELATIVE_PATH',
      'HAVE_WEBRTC_VIDEO',
      'HAVE_WEBRTC_VOICE',
      'NO_SOUND_SYSTEM',
    ],
    'configurations': {
      'Debug': {
        'defines': [
          # TODO(sergeyu): Fix libjingle to use NDEBUG instead of
          # _DEBUG and remove this define. See below as well.
          '_DEBUG',
        ],
      }
    },
    'include_dirs': [
      './overrides',
      './source',
      '../../third_party/libyuv/include',
    ],
    'dependencies': [
      '<(DEPTH)/base/base.gyp:base',
      '<(DEPTH)/net/net.gyp:net',
      '<(DEPTH)/third_party/expat/expat.gyp:expat',
    ],
    'export_dependent_settings': [
      '<(DEPTH)/third_party/expat/expat.gyp:expat',
    ],
    'direct_dependent_settings': {
      'include_dirs': [
        './overrides',
        './source',
      ],
      'defines': [
        'FEATURE_ENABLE_SSL',
        'FEATURE_ENABLE_VOICEMAIL',
        'EXPAT_RELATIVE_PATH',
        'JSONCPP_RELATIVE_PATH',
        'WEBRTC_RELATIVE_PATH',
        'NO_SOUND_SYSTEM',
      ],
      'conditions': [
        ['OS=="win"', {
          'link_settings': {
            'libraries': [
              '-lsecur32.lib',
              '-lcrypt32.lib',
              '-liphlpapi.lib',
            ],
          },
        }],
        ['OS=="win"', {
          'include_dirs': [
            '../third_party/platformsdk_win7/files/Include',
          ],
          'defines': [
              '_CRT_SECURE_NO_WARNINGS',  # Suppres warnings about _vsnprinf
          ],
        }],
        ['OS=="linux"', {
          'defines': [
            'LINUX',
          ],
        }],
        ['OS=="mac"', {
          'defines': [
            'OSX',
          ],
        }],
        ['OS=="android"', {
          'defines': [
            'ANDROID',
          ],
        }],
        ['os_posix == 1', {
          'defines': [
            'POSIX',
          ],
        }],
        ['os_bsd==1', {
          'defines': [
            'BSD',
          ],
        }],
        ['OS=="openbsd"', {
          'defines': [
            'OPENBSD',
          ],
        }],
        ['OS=="freebsd"', {
          'defines': [
            'FREEBSD',
          ],
        }],
      ],
    },
    'all_dependent_settings': {
      'configurations': {
        'Debug': {
          'defines': [
            # TODO(sergeyu): Fix libjingle to use NDEBUG instead of
            # _DEBUG and remove this define. See above as well.
            '_DEBUG',
          ],
        }
      },
    },
    'conditions': [
      ['OS=="win"', {
        'include_dirs': [
          '../third_party/platformsdk_win7/files/Include',
        ],
      }],
      ['OS=="linux"', {
        'defines': [
          'LINUX',
        ],
      }],
      ['OS=="mac"', {
        'defines': [
          'OSX',
        ],
      }],
      ['os_posix == 1', {
        'defines': [
          'POSIX',
        ],
      }],
      ['os_bsd==1', {
        'defines': [
          'BSD',
        ],
      }],
      ['OS=="openbsd"', {
        'defines': [
          'OPENBSD',
        ],
      }],
      ['OS=="freebsd"', {
        'defines': [
          'FREEBSD',
        ],
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'libjingle',
      'type': 'static_library',
      'sources': [
        'overrides/talk/base/basictypes.h',
        'overrides/talk/base/constructormagic.h',

        # Overrides logging.h/.cc because libjingle logging should be done to
        # the same place as the chromium logging.
        'overrides/talk/base/logging.cc',
        'overrides/talk/base/logging.h',

        'overrides/talk/base/scoped_ptr.h',

        'source/talk/base/Equifax_Secure_Global_eBusiness_CA-1.h',
        'source/talk/base/asyncfile.cc',
        'source/talk/base/asyncfile.h',
        'source/talk/base/asynchttprequest.cc',
        'source/talk/base/asynchttprequest.h',
        'source/talk/base/asyncpacketsocket.h',
        'source/talk/base/asyncsocket.cc',
        'source/talk/base/asyncsocket.h',
        'source/talk/base/asynctcpsocket.cc',
        'source/talk/base/asynctcpsocket.h',
        'source/talk/base/asyncudpsocket.cc',
        'source/talk/base/asyncudpsocket.h',
        'source/talk/base/autodetectproxy.cc',
        'source/talk/base/autodetectproxy.h',
        'source/talk/base/base64.cc',
        'source/talk/base/base64.h',
        'source/talk/base/basicdefs.h',
        'source/talk/base/basicpacketsocketfactory.cc',
        'source/talk/base/basicpacketsocketfactory.h',
        'source/talk/base/bytebuffer.cc',
        'source/talk/base/bytebuffer.h',
        'source/talk/base/byteorder.h',
        'source/talk/base/checks.cc',
        'source/talk/base/checks.h',
        'source/talk/base/common.cc',
        'source/talk/base/common.h',
        'source/talk/base/criticalsection.h',
        'source/talk/base/cryptstring.h',
        'source/talk/base/diskcache.cc',
        'source/talk/base/diskcache.h',
        'source/talk/base/event.cc',
        'source/talk/base/event.h',
        'source/talk/base/fileutils.cc',
        'source/talk/base/fileutils.h',
        'source/talk/base/firewallsocketserver.cc',
        'source/talk/base/firewallsocketserver.h',
        'source/talk/base/flags.cc',
        'source/talk/base/flags.h',
        'source/talk/base/helpers.cc',
        'source/talk/base/helpers.h',
        'source/talk/base/host.cc',
        'source/talk/base/host.h',
        'source/talk/base/httpbase.cc',
        'source/talk/base/httpbase.h',
        'source/talk/base/httpclient.h',
        'source/talk/base/httpclient.cc',
        'source/talk/base/httpcommon-inl.h',
        'source/talk/base/httpcommon.cc',
        'source/talk/base/httpcommon.h',
        'source/talk/base/httprequest.cc',
        'source/talk/base/httprequest.h',
        'source/talk/base/ipaddress.cc',
        'source/talk/base/ipaddress.h',
        'source/talk/base/json.cc',
        'source/talk/base/json.h',
        'source/talk/base/linked_ptr.h',
        'source/talk/base/md5.h',
        'source/talk/base/md5c.c',
        'source/talk/base/messagehandler.cc',
        'source/talk/base/messagehandler.h',
        'source/talk/base/messagequeue.cc',
        'source/talk/base/messagequeue.h',
        'source/talk/base/nethelpers.cc',
        'source/talk/base/nethelpers.h',
        'source/talk/base/network.cc',
        'source/talk/base/network.h',
        'source/talk/base/pathutils.cc',
        'source/talk/base/pathutils.h',
        'source/talk/base/physicalsocketserver.cc',
        'source/talk/base/physicalsocketserver.h',
        'source/talk/base/proxydetect.cc',
        'source/talk/base/proxydetect.h',
        'source/talk/base/proxyinfo.cc',
        'source/talk/base/proxyinfo.h',
        'source/talk/base/ratetracker.cc',
        'source/talk/base/ratetracker.h',
        'source/talk/base/sec_buffer.h',
        'source/talk/base/signalthread.cc',
        'source/talk/base/signalthread.h',
        'source/talk/base/sigslot.h',
        'source/talk/base/sigslotrepeater.h',
        'source/talk/base/socket.h',
        'source/talk/base/socketadapters.cc',
        'source/talk/base/socketadapters.h',
        'source/talk/base/socketaddress.cc',
        'source/talk/base/socketaddress.h',
        'source/talk/base/socketaddresspair.cc',
        'source/talk/base/socketaddresspair.h',
        'source/talk/base/socketfactory.h',
        'source/talk/base/socketpool.cc',
        'source/talk/base/socketpool.h',
        'source/talk/base/socketserver.h',
        'source/talk/base/socketstream.cc',
        'source/talk/base/socketstream.h',
        'source/talk/base/ssladapter.cc',
        'source/talk/base/ssladapter.h',
        'source/talk/base/sslsocketfactory.cc',
        'source/talk/base/sslsocketfactory.h',
        'source/talk/base/stream.cc',
        'source/talk/base/stream.h',
        'source/talk/base/stringdigest.cc',
        'source/talk/base/stringdigest.h',
        'source/talk/base/stringencode.cc',
        'source/talk/base/stringencode.h',
        'source/talk/base/stringutils.cc',
        'source/talk/base/stringutils.h',
        'source/talk/base/task.cc',
        'source/talk/base/task.h',
        'source/talk/base/taskparent.cc',
        'source/talk/base/taskparent.h',
        'source/talk/base/taskrunner.cc',
        'source/talk/base/taskrunner.h',
        'source/talk/base/thread.cc',
        'source/talk/base/thread.h',
        'source/talk/base/timeutils.cc',
        'source/talk/base/timeutils.h',
        'source/talk/base/timing.cc',
        'source/talk/base/timing.h',
        'source/talk/base/urlencode.cc',
        'source/talk/base/urlencode.h',
        'source/talk/base/worker.cc',
        'source/talk/base/worker.h',
        'source/talk/xmllite/qname.cc',
        'source/talk/xmllite/qname.h',
        'source/talk/xmllite/xmlbuilder.cc',
        'source/talk/xmllite/xmlbuilder.h',
        'source/talk/xmllite/xmlconstants.cc',
        'source/talk/xmllite/xmlconstants.h',
        'source/talk/xmllite/xmlelement.cc',
        'source/talk/xmllite/xmlelement.h',
        'source/talk/xmllite/xmlnsstack.cc',
        'source/talk/xmllite/xmlnsstack.h',
        'source/talk/xmllite/xmlparser.cc',
        'source/talk/xmllite/xmlparser.h',
        'source/talk/xmllite/xmlprinter.cc',
        'source/talk/xmllite/xmlprinter.h',
        'source/talk/xmpp/asyncsocket.h',
        'source/talk/xmpp/constants.cc',
        'source/talk/xmpp/constants.h',
        'source/talk/xmpp/jid.cc',
        'source/talk/xmpp/jid.h',
        'source/talk/xmpp/plainsaslhandler.h',
        'source/talk/xmpp/prexmppauth.h',
        'source/talk/xmpp/saslcookiemechanism.h',
        'source/talk/xmpp/saslhandler.h',
        'source/talk/xmpp/saslmechanism.cc',
        'source/talk/xmpp/saslmechanism.h',
        'source/talk/xmpp/saslplainmechanism.h',
        'source/talk/xmpp/xmppclient.cc',
        'source/talk/xmpp/xmppclient.h',
        'source/talk/xmpp/xmppclientsettings.h',
        'source/talk/xmpp/xmppengine.h',
        'source/talk/xmpp/xmppengineimpl.cc',
        'source/talk/xmpp/xmppengineimpl.h',
        'source/talk/xmpp/xmppengineimpl_iq.cc',
        'source/talk/xmpp/xmpplogintask.cc',
        'source/talk/xmpp/xmpplogintask.h',
        'source/talk/xmpp/xmppstanzaparser.cc',
        'source/talk/xmpp/xmppstanzaparser.h',
        'source/talk/xmpp/xmpptask.cc',
        'source/talk/xmpp/xmpptask.h',
      ],
      'dependencies': [
        '<(DEPTH)/third_party/jsoncpp/jsoncpp.gyp:jsoncpp',
      ],
      'export_dependent_settings': [
        '<(DEPTH)/third_party/jsoncpp/jsoncpp.gyp:jsoncpp',
      ],
      'conditions': [
        ['OS=="win"', {
          'sources': [
            'overrides/talk/base/win32socketinit.cc',
            'source/talk/base/schanneladapter.cc',
            'source/talk/base/schanneladapter.h',
            'source/talk/base/win32.h',
            'source/talk/base/win32.cc',
            'source/talk/base/win32filesystem.cc',
            'source/talk/base/win32filesystem.h',
            'source/talk/base/win32window.h',
            'source/talk/base/win32window.cc',
            'source/talk/base/win32securityerrors.cc',
            'source/talk/base/winfirewall.cc',
            'source/talk/base/winfirewall.h',
            'source/talk/base/winping.cc',
            'source/talk/base/winping.h',
          ],
        }],
        ['os_posix == 1', {
          'sources': [
            'source/talk/base/sslstreamadapter.cc',
            'source/talk/base/sslstreamadapter.h',
            # TODO(mallinath): Temporary fix for Android, remove when newer
            # libjingle release is rolled.
            'overrides/talk/base/unixfilesystem.cc',
            'source/talk/base/unixfilesystem.h',
          ],
        }],
        ['OS=="linux"', {
          'sources': [
            'source/talk/base/latebindingsymboltable.cc',
            'source/talk/base/latebindingsymboltable.h',
            'source/talk/base/linux.cc',
            'source/talk/base/linux.h',
          ],
        }],
        ['OS=="mac"', {
          'sources': [
            'source/talk/base/macconversion.cc',
            'source/talk/base/macconversion.h',
            'source/talk/base/macutils.cc',
            'source/talk/base/macutils.h',
          ],
        }],
        ['OS=="android"', {
          'sources!': [
            # These depend on jsoncpp which we don't load because we probably
            # don't actually need this code at all.
            'source/talk/base/json.cc',
            'source/talk/base/json.h',
          ],
          'dependencies!': [
            '<(DEPTH)/third_party/jsoncpp/jsoncpp.gyp:jsoncpp',
          ],
        }],
      ],
    },  # target libjingle
    # This has to be is a separate project due to a bug in MSVS:
    # https://connect.microsoft.com/VisualStudio/feedback/details/368272/duplicate-cpp-filename-in-c-project-visual-studio-2008
    # We have two files named "constants.cc" and MSVS doesn't handle this
    # properly.
    {
      'target_name': 'libjingle_p2p',
      'type': 'static_library',
      'sources': [
        'source/talk/p2p/base/candidate.h',
        'source/talk/p2p/base/common.h',
        'source/talk/p2p/base/constants.cc',
        'source/talk/p2p/base/constants.h',
        'source/talk/p2p/base/p2ptransport.cc',
        'source/talk/p2p/base/p2ptransport.h',
        'source/talk/p2p/base/p2ptransportchannel.cc',
        'source/talk/p2p/base/p2ptransportchannel.h',
        'source/talk/p2p/base/port.cc',
        'source/talk/p2p/base/port.h',
        'source/talk/p2p/base/portallocator.h',
        'source/talk/p2p/base/portallocator.cc',
        'source/talk/p2p/base/portallocatorsessionproxy.cc',
        'source/talk/p2p/base/portallocatorsessionproxy.h',
        'source/talk/p2p/base/portproxy.cc',
        'source/talk/p2p/base/portproxy.h',
        'source/talk/p2p/base/pseudotcp.cc',
        'source/talk/p2p/base/pseudotcp.h',
        'source/talk/p2p/base/rawtransport.cc',
        'source/talk/p2p/base/rawtransport.h',
        'source/talk/p2p/base/rawtransportchannel.cc',
        'source/talk/p2p/base/rawtransportchannel.h',
        'source/talk/p2p/base/relayport.cc',
        'source/talk/p2p/base/relayport.h',
        'source/talk/p2p/base/session.cc',
        'source/talk/p2p/base/session.h',
        'source/talk/p2p/base/sessionclient.h',
        'source/talk/p2p/base/sessiondescription.cc',
        'source/talk/p2p/base/sessiondescription.h',
        'source/talk/p2p/base/sessionid.h',
        'source/talk/p2p/base/sessionmanager.cc',
        'source/talk/p2p/base/sessionmanager.h',
        'source/talk/p2p/base/sessionmessages.cc',
        'source/talk/p2p/base/sessionmessages.h',
        'source/talk/p2p/base/parsing.cc',
        'source/talk/p2p/base/parsing.h',
        'source/talk/p2p/base/stun.cc',
        'source/talk/p2p/base/stun.h',
        'source/talk/p2p/base/stunport.cc',
        'source/talk/p2p/base/stunport.h',
        'source/talk/p2p/base/stunrequest.cc',
        'source/talk/p2p/base/stunrequest.h',
        'source/talk/p2p/base/tcpport.cc',
        'source/talk/p2p/base/tcpport.h',
        'source/talk/p2p/base/transport.cc',
        'source/talk/p2p/base/transport.h',
        'source/talk/p2p/base/transportchannel.cc',
        'source/talk/p2p/base/transportchannel.h',
        'source/talk/p2p/base/transportchannelimpl.h',
        'source/talk/p2p/base/transportchannelproxy.cc',
        'source/talk/p2p/base/transportchannelproxy.h',
        'source/talk/p2p/base/udpport.cc',
        'source/talk/p2p/base/udpport.h',
        'source/talk/p2p/client/basicportallocator.cc',
        'source/talk/p2p/client/basicportallocator.h',
        'source/talk/p2p/client/httpportallocator.cc',
        'source/talk/p2p/client/httpportallocator.h',
        'source/talk/p2p/client/sessionmanagertask.h',
        'source/talk/p2p/client/sessionsendtask.h',
        'source/talk/p2p/client/socketmonitor.cc',
        'source/talk/p2p/client/socketmonitor.h',
        'source/talk/session/tunnel/pseudotcpchannel.cc',
        'source/talk/session/tunnel/pseudotcpchannel.h',
        'source/talk/session/tunnel/tunnelsessionclient.cc',
        'source/talk/session/tunnel/tunnelsessionclient.h',
      ],
      'dependencies': [
        'libjingle',
      ],
    },  # target libjingle_p2p
    {
      'target_name': 'libjingle_peerconnection',
      'type': 'static_library',
      'sources': [
        'source/talk/app/webrtc/audiotrackimpl.cc',
        'source/talk/app/webrtc/audiotrackimpl.h',
        'source/talk/app/webrtc/mediastream.h',
        'source/talk/app/webrtc/mediastreamhandler.cc',
        'source/talk/app/webrtc/mediastreamhandler.h',
        'source/talk/app/webrtc/mediastreamimpl.cc',
        'source/talk/app/webrtc/mediastreamimpl.h',
        'source/talk/app/webrtc/mediastreamprovider.h',
        'source/talk/app/webrtc/mediastreamproxy.cc',
        'source/talk/app/webrtc/mediastreamproxy.h',
        'source/talk/app/webrtc/mediastreamtrackproxy.cc',
        'source/talk/app/webrtc/mediastreamtrackproxy.h',
        'source/talk/app/webrtc/mediatrackimpl.h',
        'source/talk/app/webrtc/notifierimpl.h',
        'source/talk/app/webrtc/peerconnectionfactoryimpl.cc',
        'source/talk/app/webrtc/peerconnectionfactoryimpl.h',
        'source/talk/app/webrtc/peerconnectionimpl.cc',
        'source/talk/app/webrtc/peerconnectionimpl.h',
        'source/talk/app/webrtc/peerconnectionsignaling.cc',
        'source/talk/app/webrtc/peerconnectionsignaling.h',
        'source/talk/app/webrtc/portallocatorfactory.cc',
        'source/talk/app/webrtc/portallocatorfactory.h',
        'source/talk/app/webrtc/roaperrorcodes.h',
        'source/talk/app/webrtc/roapmessages.cc',
        'source/talk/app/webrtc/roapmessages.h',
        'source/talk/app/webrtc/roapsession.cc',
        'source/talk/app/webrtc/roapsession.h',
        'source/talk/app/webrtc/sessiondescriptionprovider.h',
        'source/talk/app/webrtc/streamcollectionimpl.h',
        'source/talk/app/webrtc/videorendererimpl.cc',
        'source/talk/app/webrtc/videotrackimpl.cc',
        'source/talk/app/webrtc/videotrackimpl.h',
        'source/talk/app/webrtc/webrtcjson.cc',
        'source/talk/app/webrtc/webrtcjson.h',
        'source/talk/app/webrtc/webrtcsdp.cc',
        'source/talk/app/webrtc/webrtcsdp.h',
        'source/talk/app/webrtc/webrtcsession.cc',
        'source/talk/app/webrtc/webrtcsession.h',
        'source/talk/app/webrtc/webrtcsessionobserver.h',
        'source/talk/session/phone/audiomonitor.cc',
        'source/talk/session/phone/audiomonitor.h',
        'source/talk/session/phone/call.cc',
        'source/talk/session/phone/call.h',
        'source/talk/session/phone/channel.cc',
        'source/talk/session/phone/channel.h',
        'source/talk/session/phone/channelmanager.cc',
        'source/talk/session/phone/channelmanager.h',
        'source/talk/session/phone/codec.cc',
        'source/talk/session/phone/codec.h',
        'source/talk/session/phone/cryptoparams.h',
        'source/talk/session/phone/currentspeakermonitor.cc',
        'source/talk/session/phone/currentspeakermonitor.h',
        'source/talk/session/phone/devicemanager.cc',
        'source/talk/session/phone/devicemanager.h',
        'source/talk/session/phone/dummydevicemanager.cc',
        'source/talk/session/phone/dummydevicemanager.h',
        'source/talk/session/phone/filemediaengine.cc',
        'source/talk/session/phone/filemediaengine.h',
        'source/talk/session/phone/mediachannel.h',
        'source/talk/session/phone/mediaengine.cc',
        'source/talk/session/phone/mediaengine.h',
        'source/talk/session/phone/mediamessages.cc',
        'source/talk/session/phone/mediamessages.h',
        'source/talk/session/phone/mediamonitor.cc',
        'source/talk/session/phone/mediamonitor.h',
        'source/talk/session/phone/mediasession.cc',
        'source/talk/session/phone/mediasession.h',
        'source/talk/session/phone/mediasessionclient.cc',
        'source/talk/session/phone/mediasessionclient.h',
        'source/talk/session/phone/mediasink.h',
        'source/talk/session/phone/rtcpmuxfilter.cc',
        'source/talk/session/phone/rtcpmuxfilter.h',
        'source/talk/session/phone/rtpdump.cc',
        'source/talk/session/phone/rtpdump.h',
        'source/talk/session/phone/rtputils.cc',
        'source/talk/session/phone/rtputils.h',
        'source/talk/session/phone/soundclip.cc',
        'source/talk/session/phone/soundclip.h',
        'source/talk/session/phone/srtpfilter.cc',
        'source/talk/session/phone/srtpfilter.h',
        'source/talk/session/phone/ssrcmuxfilter.cc',
        'source/talk/session/phone/ssrcmuxfilter.h',
        'source/talk/session/phone/streamparams.cc',
        'source/talk/session/phone/videocapturer.cc',
        'source/talk/session/phone/videocapturer.h',
        'source/talk/session/phone/videocommon.cc',
        'source/talk/session/phone/videocommon.h',
        'source/talk/session/phone/videoframe.cc',
        'source/talk/session/phone/videoframe.h',
        'source/talk/session/phone/voicechannel.h',
        'source/talk/session/phone/webrtccommon.h',
        'source/talk/session/phone/webrtcpassthroughrender.cc',
        'source/talk/session/phone/webrtcvideocapturer.cc',
        'source/talk/session/phone/webrtcvideocapturer.h',
        'source/talk/session/phone/webrtcvideoengine.cc',
        'source/talk/session/phone/webrtcvideoengine.h',
        'source/talk/session/phone/webrtcvideoframe.cc',
        'source/talk/session/phone/webrtcvideoframe.h',
        'source/talk/session/phone/webrtcvie.h',
        'source/talk/session/phone/webrtcvoe.h',
        'source/talk/session/phone/webrtcvoiceengine.cc',
        'source/talk/session/phone/webrtcvoiceengine.h',
      ],
      'conditions': [
        ['OS!="android"', {
          'dependencies': [
            # We won't build with WebRTC on Android.
            '<(DEPTH)/third_party/webrtc/modules/modules.gyp:video_capture_module',
            '<(DEPTH)/third_party/webrtc/modules/modules.gyp:video_render_module',
            '<(DEPTH)/third_party/webrtc/video_engine/video_engine.gyp:video_engine_core',
            '<(DEPTH)/third_party/webrtc/voice_engine/voice_engine.gyp:voice_engine_core',
            '<(DEPTH)/third_party/webrtc/system_wrappers/source/system_wrappers.gyp:system_wrappers',
            'libjingle',
            'libjingle_p2p',
          ],
        }],
      ],
    },  # target libjingle_peerconnection
  ],
}
