/*
  Copyright (C) 2014 Jouni Roivas
  Copyright (C) 2013 Jolla Ltd.
  Contact: Thomas Perl <thomas.perl@jollamobile.com>
  All rights reserved.

  You may use this file under the terms of BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the authors nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

import QtQuick 2.0
import QtQml 2.1
import Sailfish.Silica 1.0

Page {
    id: watchPage

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
        contentHeight: column.height

        VerticalScrollDecorator { flickable: flickable }

        Column {
            id: column
            width: watchPage.width

            PageHeader {
                title: pebbled.name
            }

            Row {
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Theme.paddingLarge
                }


                Button {
                    text: "Ping"
                    width: parent.width / 2
                    onClicked: {
                        pebbled.ping(66)
                    }
                }

                Button {
                    text: "Sync Time"
                    width: parent.width / 2
                    onClicked: {
                        pebbled.time()
                    }
                }
            }

            Item {
                height: Theme.paddingMedium
            }

            Label {
                text: qsTr("Installed applications")
                font.family: Theme.fontFamilyHeading
                color: Theme.highlightColor
                anchors.right: parent.right
                anchors.rightMargin: Theme.paddingMedium
            }

            Repeater {
                id: slotsRepeater
                model: pebbled.appSlots

                ListItem {
                    id: slotDelegate
                    menu: slotMenu
                    contentHeight: Theme.itemSizeSmall

                    property bool isEmptySlot: modelData === ""
                    property var appInfo: pebbled.appInfoByUuid(modelData)
                    property bool isKnownApp: appInfo.hasOwnProperty("uuid")
                    property bool busy: false

                    function configure() {
                        var uuid = modelData;
                        pebbled.launchApp(uuid);
                        console.log("going to call configure on app with uuid " + uuid);
                        var url = pebbled.configureApp(uuid);
                        console.log("received url: " + url);
                        pageStack.push(Qt.resolvedUrl("AppConfigPage.qml"), {
                                           url: url,
                                           name: appInfo.longName
                                       });
                    }

                    function remove() {
                        remorseAction(qsTr("Uninstalling"), function() {
                            busy = true;
                            pebbled.unloadApp(index);
                        });
                    }

                    function install() {
                        var dialog = pageStack.push(Qt.resolvedUrl("InstallAppDialog.qml"));
                        dialog.accepted.connect(function() {
                            var uuid = dialog.selectedUuid;

                            if (pebbled.isAppInstalled(uuid)) {
                                console.warn("uuid already installed");
                                return;
                            }

                            var slot = index;
                            console.log("installing " + uuid + " into " + slot);
                            busy = true;
                            pebbled.uploadApp(uuid, slot);
                        });

                    }

                    Image {
                        id: slotImage
                        anchors {
                            top: parent.top
                            left: parent.left
                            leftMargin: Theme.paddingLarge
                        }
                        width: Theme.itemSizeSmall
                    }

                    BusyIndicator {
                        id: slotBusy
                        anchors.centerIn: slotImage
                        running: slotDelegate.busy
                    }

                    Label {
                        id: slotName
                        anchors {
                            left: slotImage.right
                            leftMargin: Theme.paddingMedium
                            right: parent.right
                            rightMargin: Theme.paddiumLarge
                            verticalCenter: parent.verticalCenter
                        }
                        text: isEmptySlot ? qsTr("(empty slot)") : (isKnownApp ? appInfo.longName : qsTr("(slot in use by unknown app)"))
                        color: slotDelegate.highlighted ? Theme.highlightColor : Theme.primaryColor
                        onTextChanged: slotDelegate.busy = false;
                    }

                    Component {
                        id: slotMenu
                        ContextMenu {
                            MenuItem {
                                text: qsTr("Configure...")
                                visible: !isEmptySlot && isKnownApp
                                onClicked: configure();
                            }
                            MenuItem {
                                text: qsTr("Uninstall")
                                visible: !isEmptySlot
                                onClicked: remove();
                            }
                        }
                    }

                    onClicked: {
                        if (isEmptySlot) {
                            install();
                        } else {
                            showMenu();
                        }
                    }
                }
            }
        }
    }
}
