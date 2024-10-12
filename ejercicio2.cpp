#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

// Saldo inicial de la cuenta bancaria
double saldo = 100000.00;

// Semaforo para controlar el acceso al cajero
sem_t semaforo;

// Función que ejecuta cada hilo
void* retirarDinero(void* arg) {
    double* monto = (double*)arg;

    // Esperar para acceder al cajero
    sem_wait(&semaforo);

    cout << "Cliente intentando retirar Q" << *monto << endl;

    // Verificar si hay saldo suficiente
    if (saldo >= *monto) {
        saldo -= *monto;
        cout << "Retiro exitoso. Saldo restante: Q" << saldo << endl;
    } else {
        cout << "Saldo insuficiente. No se pudo completar la transaccion." << endl;
    }


    sem_post(&semaforo);

    return nullptr;
}

int main() {
    int numClientes;

    cout << "Ingrese la cantidad de clientes: ";
    cin >> numClientes;

    // Crear arrays dinámicos para los hilos y los montos
    pthread_t* clientes = new pthread_t[numClientes];
    double* montos = new double[numClientes];

    // Inicializar el semáforo con un valor de 1
    sem_init(&semaforo, 0, 1);

    // Solicitar los montos de retiro para cada cliente
    for (int i = 0; i < numClientes; ++i) {
        cout << "Ingrese el monto que el cliente " << i + 1 << " intentara retirar: ";
        cin >> montos[i];
    }

    // Crear los hilos para los clientes
    for (int i = 0; i < numClientes; ++i) {
        pthread_create(&clientes[i], nullptr, retirarDinero, (void*)&montos[i]);
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < numClientes; ++i) {
        pthread_join(clientes[i], nullptr);
    }

    sem_destroy(&semaforo);

    delete[] clientes;
    delete[] montos;

    cout << "Saldo final: Q" << saldo << endl;

    return 0;
}