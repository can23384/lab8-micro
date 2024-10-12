#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <ctime>

using namespace std;

const char* productos[] = {"Pata", "Respaldo", "Asiento"};
const int numProductos = 3;
const int MAX_BUFFER = 10;
const int MAX_SILLAS = 3;

int buffer[MAX_BUFFER];
int in = 0;
int out = 0;

int piezasEnBuffer[numProductos] = {0};

int sillasProducidas = 0;
bool produccionActiva = true;

sem_t vacios;   
sem_t llenos;   
pthread_mutex_t mutex;

void* productor(void* arg) {
    int id = *(int*)arg;
    int piezaId;
    
    while (true) {
        pthread_mutex_lock(&mutex);
        bool continuar = produccionActiva;
        pthread_mutex_unlock(&mutex);

        if (!continuar) {
            break;
        }

        piezaId = rand() % numProductos;

        sem_wait(&vacios);
        pthread_mutex_lock(&mutex);

        if (!produccionActiva) {
            pthread_mutex_unlock(&mutex);
            sem_post(&vacios);
            break;
        }

        buffer[in] = piezaId;
        piezasEnBuffer[piezaId]++;
        cout << "Productor " << id << " ha fabricado la pieza " << productos[piezaId]
             << " y la coloco en la posicion " << in << endl;
        in = (in + 1) % MAX_BUFFER;

        pthread_mutex_unlock(&mutex);
        sem_post(&llenos);
        
        sleep(1);
    }
    
    cout << "Productor " << id << " ha terminado su trabajo." << endl;
    return NULL;
}

void* consumidor(void* arg) {
    int id = *(int*)arg;
    int piezaId;
    int piezasNecesarias[numProductos] = {4, 1, 1};
    int piezasDisponibles[numProductos] = {0};

    while (true) {
        pthread_mutex_lock(&mutex);
        if (sillasProducidas >= MAX_SILLAS) {
            produccionActiva = false;
            pthread_mutex_unlock(&mutex);

            sem_post(&vacios);
            sem_post(&llenos);
            break;
        }
        pthread_mutex_unlock(&mutex);

        sem_wait(&llenos);
        pthread_mutex_lock(&mutex);

        piezaId = buffer[out];
        piezasEnBuffer[piezaId]--;
        cout << "Consumidor " << id << " ha retirado la pieza " << productos[piezaId]
             << " de la posicion " << out << endl;
        out = (out + 1) % MAX_BUFFER;

        piezasDisponibles[piezaId]++;

        bool puedeEnsamblar = true;
        for (int i = 0; i < numProductos; ++i) {
            if (piezasDisponibles[i] < piezasNecesarias[i]) {
                puedeEnsamblar = false;
                break;
            }
        }

        if (puedeEnsamblar) {
            for (int i = 0; i < numProductos; ++i) {
                piezasDisponibles[i] -= piezasNecesarias[i];
            }
            sillasProducidas++;
            cout << "Consumidor " << id << " ha ensamblado una silla completa. Sillas ensambladas: "
                 << sillasProducidas << "/" << MAX_SILLAS << endl;
        }

        pthread_mutex_unlock(&mutex);
        sem_post(&vacios);
        
        sleep(2);
    }

    cout << "Consumidor " << id << " ha terminado su trabajo." << endl;
    return NULL;
}

int main() {
    int numProductores, numConsumidores;

    cout << "Ingrese el numero de productores: ";
    cin >> numProductores;
    cout << "Ingrese el numero de consumidores: ";
    cin >> numConsumidores;

    pthread_t productores[100], consumidores[100];  
    int idProductores[100], idConsumidores[100];    

    sem_init(&vacios, 0, MAX_BUFFER);  
    sem_init(&llenos, 0, 0);           
    pthread_mutex_init(&mutex, NULL);

    srand(time(NULL));

    for (int i = 0; i < numProductores; ++i) {
        idProductores[i] = i + 1;
        pthread_create(&productores[i], NULL, productor, &idProductores[i]);
    }

    for (int i = 0; i < numConsumidores; ++i) {
        idConsumidores[i] = i + 1;
        pthread_create(&consumidores[i], NULL, consumidor, &idConsumidores[i]);
    }

    for (int i = 0; i < numConsumidores; ++i) {
        pthread_join(consumidores[i], NULL);
    }

    for (int i = 0; i < numProductores; ++i) {
        pthread_join(productores[i], NULL);
    }

    cout << "\n--- Reporte Final ---\n";
    cout << "Sillas fabricadas en total: " << sillasProducidas << endl;
    cout << "Piezas sobrantes en el almacen:\n";
    int piezasRestantes[numProductos] = {0};

    pthread_mutex_lock(&mutex);
    int index = out;
    int count = 0;
    int itemsEnBuffer = (in - out + MAX_BUFFER) % MAX_BUFFER;
    while (count < itemsEnBuffer) {
        int piezaId = buffer[index];
        piezasRestantes[piezaId]++;
        index = (index + 1) % MAX_BUFFER;
        count++;
    }
    pthread_mutex_unlock(&mutex);

    for (int i = 0; i < numProductos; ++i) {
        cout << "- " << productos[i] << ": " << piezasRestantes[i] << endl;
    }

    sem_destroy(&vacios);
    sem_destroy(&llenos);
    pthread_mutex_destroy(&mutex);

    return 0;
}
