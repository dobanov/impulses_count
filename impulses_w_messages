#include <wiringPi.h>
#include <iostream>
#include <fstream>
#include <csignal>
#include <ctime>
#include <unistd.h> // Для использования getopt
#include <curl/curl.h>
#include <vector>

// Function to read configuration from a file
bool readConfig(const std::string& configPath, std::string& botId, std::string& chatId, bool& debug) {
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
    config.close();
}

// Глобальные переменные для хранения времени последнего срабатывания
volatile unsigned long lastDebounceTime23 = 0;
volatile unsigned long lastDebounceTime17 = 0;
const unsigned long debounceDelay = 1000; // Дебаунс задержка в миллисекундах

// Глобальные переменные для хранения состояния
volatile bool state23 = false;
volatile bool state17 = false;

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

void sendTextToTelegram(const std::string& botId, const std::string& chatId, const std::string& message, bool debugMode) {
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "https://api.telegram.org/bot" + botId + "/sendMessage";
        std::string data = "chat_id=" + chatId + "&text=" + curl_easy_escape(curl, message.c_str(), 0);
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
            std::cout << "Text message sent successfully." << std::endl;
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
        sendTextToTelegram(BOT_TOKEN, CHAT_ID, message, debugMode);
    }
}

void pulseCallback23() {
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceTime23 > debounceDelay) {
        if (!state23) {
            updateFile("cold");
            state23 = true;
        }
        lastDebounceTime23 = currentTime;
    }
}

void pulseCallback17() {
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceTime17 > debounceDelay) {
        if (!state17) {
            updateFile("hot");
            state17 = true;
        }
        lastDebounceTime17 = currentTime;
    }
}

void resetState() {
    state23 = false;
    state17 = false;
}

void handleSignal(int signal) {
    std::cout << "Exiting program..." << std::endl;
    wiringPiISR(23, INT_EDGE_FALLING, NULL); // Remove interrupt handlers
    wiringPiISR(17, INT_EDGE_FALLING, NULL);
    exit(0);
}

int main(int argc, char* argv[]) {
    // Обработка параметров командной строки
    int opt;
    while ((opt = getopt(argc, argv, "shd")) != -1) {
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
            default: /* '?' */
                std::cerr << "Usage: " << argv[0] << " [-s] [-h] [-d]\n";
                return 1;
        }
    }

    if (help) {
        std::cout << "Usage: " << argv[0] << " [-s] [-h] [-d]\n"
                  << "Options:\n"
                  << "  -s        Silence mode, suppresses output\n"
                  << "  -h        Display this help message\n"
                  << "  -d        Debug mode, prints additional information\n";
        return 0;
    }

    // Initialize WiringPi and set up GPIO
    if (wiringPiSetupGpio() == -1) {
        std::cerr << "Failed to initialize WiringPi" << std::endl;
        return 1;
    }

    // Set up GPIO pins 23 and 17 as input
    pinMode(23, INPUT);
    pullUpDnControl(23, PUD_UP);

    pinMode(17, INPUT);
    pullUpDnControl(17, PUD_UP);

    // Set up ISR for GPIO pins
    if (wiringPiISR(23, INT_EDGE_FALLING, &pulseCallback23) < 0) {
        std::cerr << "Failed to set up ISR for pin 23" << std::endl;
        return 1;
    }

    if (wiringPiISR(17, INT_EDGE_FALLING, &pulseCallback17) < 0) {
        std::cerr << "Failed to set up ISR for pin 17" << std::endl;
        return 1;
    }

    // Handle termination signal to clean up
    signal(SIGINT, handleSignal);

    if (!silence) {
        std::cout << "Press Ctrl+C to exit" << std::endl;
    }

    // Read configuration from file
    std::string configPath = std::string(getenv("HOME")) + "/.config/impulseswsend.ini";
    if (!readConfig(configPath, BOT_TOKEN, CHAT_ID, debug)) {
        // Create default configuration if it does not exist
        createDefaultConfig(configPath);
        readConfig(configPath, BOT_TOKEN, CHAT_ID, debug);
    }

    // Keep the program running to listen for interrupts
    while (true) {
        // Check if the state has returned to open
        if (digitalRead(23) == HIGH) {
            state23 = false;
        }

        if (digitalRead(17) == HIGH) {
            state17 = false;
        }

        delay(100); // Sleep for 0.1 second
    }

    return 0;
}
