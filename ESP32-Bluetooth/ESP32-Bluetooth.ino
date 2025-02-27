#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <BLEScan.h>

// Adresse MAC du Muse 2 (remplace par la tienne)
static BLEAddress museAddress("00:55:DA:B5:00:F2");

// UUIDs des services et caractéristiques
static const char* GENERIC_ACCESS_UUID = "1800"; // Service Generic Access
static const char* TARGET_CHARACTERISTIC_UUID = "00002a00-0000-1000-8000-00805f9b34fb"; // Nom du Muse
static const char* UNKNOWN_SERVICE_UUID = "0000fe8d-0000-1000-8000-00805f9b34fb";  // Service EEG
static const char* EEG_CHARACTERISTIC_UUID = "273e0001-4c4d-454d-96be-f03bac821358";  // Caractéristique EEG

BLEClient* pClient = nullptr;
bool connected = false;

// Callback de réception des données EEG
static void eegCallback(BLERemoteCharacteristic* pCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    Serial.print("Notification reçue de : ");
    Serial.println(pCharacteristic->getUUID().toString().c_str());
    Serial.print("Données EEG : ");
    for (size_t i = 0; i < length; i++) {
        Serial.print(pData[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

// Fonction pour lire une caractéristique
void readCharacteristicData(BLERemoteCharacteristic* pRemoteCharacteristic) {
    if (pRemoteCharacteristic->canRead()) {
        std::string value = std::string(pRemoteCharacteristic->readValue().c_str());
        Serial.print("Valeur lue : ");
        Serial.println(value.c_str());
    } else {
        Serial.println("Lecture non autorisée pour cette caractéristique.");
    }
}

// Connexion au Muse
bool connectToMuse() {
    Serial.println("Connexion au Muse 2...");

    pClient = BLEDevice::createClient();
    if (!pClient->connect(museAddress)) {
        Serial.println("Échec de connexion !");
        return false;
    }

    Serial.println("Connecté !");

    // Recherche du service Generic Access
    BLERemoteService* pAccessService = pClient->getService(GENERIC_ACCESS_UUID);
    if (pAccessService) {
        Serial.println("Service Generic Access trouvé !");
        
        // Recherche de la caractéristique du nom du Muse
        BLERemoteCharacteristic* pNameChar = pAccessService->getCharacteristic(TARGET_CHARACTERISTIC_UUID);
        if (pNameChar) {
            Serial.println("==> Lecture du nom du Muse...");
            readCharacteristicData(pNameChar);
        } else {
            Serial.println("Caractéristique du nom non trouvée.");
        }
    } else {
        Serial.println("Service Generic Access non trouvé.");
    }

    // Recherche du service EEG
    BLERemoteService* pEEGService = pClient->getService(UNKNOWN_SERVICE_UUID);
    if (pEEGService) {
        Serial.println("Service EEG trouvé !");
        
        // Recherche de la caractéristique EEG
        BLERemoteCharacteristic* pEEGChar = pEEGService->getCharacteristic(EEG_CHARACTERISTIC_UUID);
        if (pEEGChar) {
            Serial.println("Caractéristique EEG trouvée !");
            
            if (pEEGChar->canNotify()) {
                Serial.println("Abonnement aux notifications EEG...");
                pEEGChar->registerForNotify(eegCallback);
                
                // Activer manuellement les notifications via le descripteur CCCD
                BLERemoteDescriptor* pDescriptor = pEEGChar->getDescriptor(BLEUUID((uint16_t)0x2902));
                if (pDescriptor) {
                    uint8_t notificationOn[] = {0x01, 0x00}; // Activer les notifications
                    pDescriptor->writeValue(notificationOn, 2, true);
                    Serial.println("Notifications activées !");
                } else {
                    Serial.println("Descripteur CCCD non trouvé !");
                }
            } else {
                Serial.println("Cette caractéristique ne supporte pas les notifications.");
            }
        } else {
            Serial.println("Caractéristique EEG non trouvée.");
        }
    } else {
        Serial.println("Service EEG non trouvé.");
    }

    connected = true;
    return true;
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n\n");
    Serial.println("Démarrage ESP32-S3 BLE Client...");

    BLEDevice::init("");
    
    if (!connectToMuse()) {
        Serial.println("Échec de connexion. Redémarrage...");
        delay(5000);
        ESP.restart();
    }
}

void loop() {
    delay(1000);
    if (!connected) {
        Serial.println("Connexion perdue !");
    }
}
