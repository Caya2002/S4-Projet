#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <BLEScan.h>

// Adresse MAC du Muse 2 (à remplacer par la tienne)
static BLEAddress museAddress("00:55:DA:B5:00:F2");

// UUIDs (à adapter selon les besoins)
static const char* GENERIC_ACCESS_UUID = "1800"; // Generic Access Service UUID
static const char* GENERIC_ATTRIBUTE_UUID = "1801"; // Generic Attribute Service UUID
static const char* UNKNOWN_SERVICE_UUID = "FE8D"; // Unknown Service UUID
static const char* TARGET_CHARACTERISTIC_UUID = "00002a00-0000-1000-8000-00805f9b34fb"; // UUID de la caractéristique à lire

BLEClient* pClient = nullptr;
bool connected = false;

// Fonction de connexion
bool connectToMuse() {
    Serial.println("Connexion au Muse 2...");

    pClient = BLEDevice::createClient();
    if (!pClient->connect(museAddress)) {
        Serial.println("Échec de connexion !");
        return false;
    }

    Serial.println("Connecté !");
    
    // Recherche des services
    BLERemoteService* pRemoteService = nullptr;

    // Recherche du service Generic Access
    pRemoteService = pClient->getService(GENERIC_ACCESS_UUID);
    if (pRemoteService) {
        Serial.println("Service Generic Access trouvé !");
        listCharacteristics(pRemoteService);
    } else {
        Serial.println("Service Generic Access non trouvé.");
    }

    // Recherche du service Generic Attribute
    pRemoteService = pClient->getService(GENERIC_ATTRIBUTE_UUID);
    if (pRemoteService) {
        Serial.println("Service Generic Attribute trouvé !");
        listCharacteristics(pRemoteService);
    } else {
        Serial.println("Service Generic Attribute non trouvé.");
    }

    // Recherche du service Inconnu
    pRemoteService = pClient->getService(UNKNOWN_SERVICE_UUID);
    if (pRemoteService) {
        Serial.println("Unknown Service trouvé !");
        listCharacteristics(pRemoteService);
    } else {
        Serial.println("Unknown Service non trouvé.");
    }

    connected = true;
    return true;
}

// Fonction pour lire les données d'une caractéristique spécifique
void readCharacteristicData(BLERemoteCharacteristic* pRemoteCharacteristic) {
    if (pRemoteCharacteristic->canRead()) {  // Vérifie si la lecture est autorisée
        std::string value = std::string(pRemoteCharacteristic->readValue().c_str());
        Serial.print("Valeur lue : ");
        Serial.println(value.c_str());  // Affiche la valeur en tant que chaîne de caractères
    } else {
        Serial.println("Lecture non autorisée pour cette caractéristique.");
    }
}

// Fonction pour lister les caractéristiques d'un service
void listCharacteristics(BLERemoteService* pRemoteService) {
    std::map<std::string, BLERemoteCharacteristic*>* characteristics = pRemoteService->getCharacteristics();
    
    for (auto& pair : *characteristics) {
        BLERemoteCharacteristic* pRemoteCharacteristic = pair.second;
        Serial.print("Caractéristique UUID : ");
        Serial.println(pRemoteCharacteristic->getUUID().toString().c_str());

        // Vérifie si c'est l'UUID cible et lit les données
        if (pRemoteCharacteristic->getUUID().toString() == TARGET_CHARACTERISTIC_UUID) {
            Serial.println("==> Caractéristique cible détectée, lecture des données...");
            readCharacteristicData(pRemoteCharacteristic);
        }
    }
}

void setup() {
    Serial.begin(115200);
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
