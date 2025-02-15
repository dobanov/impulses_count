#include <wiringPi.h>
#include <iostream>
#include <fstream>
#include <csignal>
#include <ctime>
#include <unistd.h> // Для использования getopt
#include <curl/curl.h>
#include <vector>
#include <sstream>

// Function to read configuration from a file
bool readConfig(const std::string& configPath, std::string& botId, std::string& chatId, bool& debug, int& pinCold, int& pinHot) {
    std::ifstream config(configPath);
    if (!config.is_open()) {
        return false;
    }
    std::string line;
    while (std::getline(config, line)) {
        if (line.find("bot_id=") != std::string::npos) {
            botId = line.substr(line.find("=") + 1);
        } else if (line.find("chat_id=") != std::string::npos) {
            chatId = line.substr(line.find("=") + 1);
        } else if (line.find("debug=") != std::string::npos) {
            debug = (line.substr(line.find("=") + 1) == "true");
        } else if (line.find("pin_cold=") != std::string::npos) {
            pinCold = std::stoi(line.substr(line.find("=") + 1));
        } else if (line.find("pin_hot=") != std::string::npos) {
            pinHot = std::stoi(line.substr(line.find("=") + 1));
        }
    }
    config.close();
    return true;
}

// Function to create a default configuration file
void createDefaultConfig(const std::string& configPath) {
    std::ofstream config(configPath);
    config << "bot_id=\n";
    config << "chat_id=\n";
    config << "debug=false\n";
    config << "pin_cold=23\n";  // Default pin for cold water
    config << "pin_hot=17\n";   // Default pin for hot water
    config.close();
}

// Глобальные переменные для хранения времени последнего срабатывания
volatile unsigned long lastDebounceTimeCold = 0;
volatile unsigned long lastDebounceTimeHot = 0;
const unsigned long debounceDelay = 1000; // Дебаунс задержка в миллисекундах

// Глобальные переменные для хранения состояния
volatile bool stateCold = false;
volatile bool stateHot = false;

// Флаги для параметров командной строки
bool silence = false;
bool help = false;
bool debugMode = false;

// Telegram bot token and chat ID
std::string BOT_TOKEN;
std::string CHAT_ID;
bool debug = false;

size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void sendTextToTelegram(const std::string& botId, const std::vector<std::string>& chatIds, const std::string& message, bool debugMode) {
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "https://api.telegram.org/bot" + botId + "/sendMessage";
        std::string escapedMessage = curl_easy_escape(curl, message.c_str(), 0);

        for (const auto& chatId : chatIds) {
            std::string data = "chat_id=" + chatId + "&text=" + escapedMessage;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);

            std::string response_string;
            std::string header_string;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "Failed to send message to Telegram: " << curl_easy_strerror(res) << std::endl;
            }

            if (debugMode) {
                std::cout << "Response: " << response_string << std::endl;
                std::cout << "Headers: " << header_string << std::endl;
                std::cout << "Text message sent successfully to chat ID: " << chatId << std::endl;
            }
        }

        curl_easy_cleanup(curl);
    }
}

void updateFile(const std::string& filename) {
    std::ifstream inFile(filename);
    int count = 0;

    if (inFile.is_open()) {
        inFile >> count;
        inFile.close();
    }

    count += 1;

    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << count;
        outFile.close();
    } else {
        std::cerr << "Failed to open " << filename << " for writing." << std::endl;
    }

    if (!silence) {
        std::string message = "Another 10 liters of " + filename + " water leaked " + std::to_string(count);

        // Split CHAT_ID into individual chatIds
        std::vector<std::string> chatIds;
        std::istringstream iss(CHAT_ID);
        std::string chatId;
        while (std::getline(iss, chatId, ',')) {
            chatIds.push_back(chatId);
        }

        sendTextToTelegram(BOT_TOKEN, chatIds, message, debugMode);
    }
}

void pulseCallbackCold() {
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceTimeCold > debounceDelay) {
        if (!stateCold) {
            updateFile("cold");
            stateCold = true;
        }
        lastDebounceTimeCold = currentTime;
    }
}

void pulseCallbackHot() {
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceTimeHot > debounceDelay) {
        if (!stateHot) {
            updateFile("hot");
            stateHot = true;
        }
        lastDebounceTimeHot = currentTime;
    }
}

void resetState() {
    stateCold = false;
    stateHot = false;
}

void handleSignal(int signal) {
    std::cout << "Exiting program..." << std::endl;
    exit(0);
}

int main(int argc, char* argv[]) {
    int pinCold = 23;  // Default pin for cold water
    int pinHot = 17;   // Default pin for hot water

    // Обработка параметров командной строки
    int opt;
    while ((opt = getopt(argc, argv, "shd:p:c:")) != -1) {
        switch (opt) {
            case 's':
                silence = true;
                break;
            case 'h':
                help = true;
                break;
            case 'd':
                debugMode = true;
                break;
            case 'p':  // Пин для холодной воды
                pinCold = std::stoi(optarg);
                break;
            case 'c':  // Пин для горячей воды
                pinHot = std::stoi(optarg);
                break;
            default: /* '?' */
                std::cerr << "Usage: " << argv[0] << " [-s] [-h] [-d] [-p pin_cold] [-c pin_hot]\n";
                return 1;
        }
    }

    if (help) {
        std::cout << "Usage: " << argv[0] << " [-s] [-h] [-d] [-p pin_cold] [-c pin_hot]\n"
                  << "Options:\n"
                  << "  -s        Silence mode, suppresses output\n"
                  << "  -h        Display this help message\n"
                  << "  -d        Debug mode, prints additional information\n"
                  << "  -p pin_cold  Set the pin number for cold water\n"
                  << "  -c pin_hot   Set the pin number for hot water\n";
        return 0;
    }

    // Initialize WiringPi and set up GPIO
    if (wiringPiSetupGpio() == -1) {
        std::cerr << "Failed to initialize WiringPi" << std::endl;
        return 1;
    }

    // Set up GPIO pins as input
    pinMode(pinCold, INPUT);
    pullUpDnControl(pinCold, PUD_UP);

    pinMode(pinHot, INPUT);
    pullUpDnControl(pinHot, PUD_UP);

    // Set up ISR for GPIO pins
    if (wiringPiISR(pinCold, INT_EDGE_FALLING, &pulseCallbackCold) < 0) {
        std::cerr << "Failed to set up ISR for pin " << pinCold << std::endl;
        return 1;
    }

    if (wiringPiISR(pinHot, INT_EDGE_FALLING, &pulseCallbackHot) < 0) {
        std::cerr << "Failed to set up ISR for pin " << pinHot << std::endl;
        return 1;
    }

    // Handle termination signal to clean up
    signal(SIGINT, handleSignal);

    // Read configuration from file
    std::string configPath = std::string(getenv("HOME")) + "/.config/impulseswsend.ini";
    if (!readConfig(configPath, BOT_TOKEN, CHAT_ID, debug, pinCold, pinHot)) {
        // Create default configuration if it does not exist
        createDefaultConfig(configPath);
        readConfig(configPath, BOT_TOKEN, CHAT_ID, debug, pinCold, pinHot);
    }

    if (!silence) {
        std::cout << "Press Ctrl+C to exit" << std::endl;
    }

    // Keep the program running to listen for interrupts
    while (true) {
        // Check if the state has returned to open
        if (digitalRead(pinCold) == HIGH) {
            stateCold = false;
        }

        if (digitalRead(pinHot) == HIGH) {
            stateHot = false;
        }

        delay(3000); // Sleep for 3 seconds
    }

    return 0;
}
