#include <MACPool.hpp>

MACPool::MACPool(String mac, int signal, unsigned long time, bool newMAC) {
    this->mac = mac;
    this->signal = signal;
    this->time = time;
    this->newMAC = newMAC;
}

String MACPool::getMAC() {
    return this->mac;
}

int MACPool::getSignal() {
    return this->signal;
}

unsigned long MACPool::getTime() {
    return this->time;
}

void MACPool::updateTime(unsigned long time) {
    this->time = time;
}

void MACPool::updateNewMAC(bool nm) {
    this->newMAC = nm;
}

bool MACPool::getNewMAC() {
    return this->newMAC;
}