#include <wiringPi.h>
#include <iostream>
#include <fstream>
#include <csignal>
#include <ctime>

// Глобальные переменные для хранения времени последнего срабатывания
volatile unsigned long lastDebounceTime23 = 0;
volatile unsigned long lastDebounceTime17 = 0;
const unsigned long debounceDelay = 200; // Дебаунс задержка в миллисекундах

// Глобальные переменные для хранения состояния
volatile bool state23 = false;
volatile bool state17 = false;

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

    std::cout << "Updated " << filename << " to " << count << std::endl;
}

void pulseCallback23() {
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceTime23 > debounceDelay) {
        if (!state23) {
            updateFile("cold.txt");
            state23 = true;
        }
        lastDebounceTime23 = currentTime;
    }
}

void pulseCallback17() {
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceTime17 > debounceDelay) {
        if (!state17) {
            updateFile("hot.txt");
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

int main() {
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

    std::cout << "Press Ctrl+C to exit" << std::endl;

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

