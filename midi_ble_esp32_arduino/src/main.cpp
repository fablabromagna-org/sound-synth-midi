/**
 * ITLab BLE MIDI Board
 * Copyright (C) 2020 Ivan Tarozzi (itarozzi@gmail.com) 
 * 
 *
 * A BLE (Bluetooth Low Energy) MIDI implementation based on ESP32
 * 
 * 
 * -----------------------------------------------------------------
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Credits:
 * -------- 
 * Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
 * Ported to Arduino ESP32 by Evandro Copercini
 *  updates by chegewara
*/

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#define SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"  // The MIDI Service
#define CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"  // The MIDI Characteristic

const int midi_channel = 1;


BLECharacteristic *pCharacteristic;
int cc_code_prev = 0;


uint8_t midiPacket[] = {
   0x80,  // header
   0x80,  // timestamp, not implemented 
   0x00,  // status
   0x3c,  // data - key note: 0x3c == 60 == middle c
   0x00   // velocity
};


void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");


  /** Init the BLE Service **/ 
  BLEDevice::init("IT ESP32 MIDI ");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_WRITE_NR
                                       );

  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {


  /** Send the CC MIDI message ON **/
  midiPacket[2] = 0x90 + midi_channel -1; // Note ON  on channel midi_channel
  midiPacket[3] = 60; // note
  midiPacket[4] = 100;    // velocity
  Serial.println("sending MIDI NOTE ON ...");

  pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes
  pCharacteristic->notify();
  vTaskDelay(1000/portTICK_PERIOD_MS);


  /** Send the CC MIDI message OFF **/
  midiPacket[2] = 0x80 + midi_channel - 1; // CC on channel midi_channel
  midiPacket[3] = 60; // Note
  midiPacket[4] = 0;         // velocity
  Serial.println("sending MIDI NOTE OFF ...");

  pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes
  pCharacteristic->notify();
  vTaskDelay(1000/portTICK_PERIOD_MS);

}
