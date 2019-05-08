#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include <chrono>
#include <unistd.h>
#include "Structs.cpp"
#include "Funciones.cpp"

std::vector<std::thread> v_entradas;
std::vector<std::thread> cola_comida;
std::vector<solicitud> v_solicitudes;
std::vector<pago> v_pagos;
std::mutex sem_pago;
std::mutex sem_reserva;
std::mutex sem_taquilla;
std::mutex sem_aux;
std::mutex sem_aux2;

void realizarPago()
{
    pago p = v_pagos.front();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Cliente: " << p.id_cliente << " realiza el pago de " << p.tipo_pago << "." << '\n';
    v_pagos.erase(v_pagos.begin());
}

void solicitarPago(int id_hilo, std::string tipo)
{
    pago p;
    p.id_cliente = id_hilo;
    p.tipo_pago = tipo;
    sem_aux2.lock();
    v_pagos.insert(v_pagos.begin(), p);
    sem_aux2.unlock();
    sem_pago.unlock();
}

void reservarHueco()
{
    int reserva_inicial = -1;

    solicitud s = v_solicitudes.front();
    v_solicitudes.erase(v_solicitudes.begin());
    sem_reserva.lock();
    reserva_inicial = comprobarReserva(s.asientos_cliente);
    sem_reserva.unlock();

    if (reserva_inicial == -1)
    {
        std::cout << "No hay suficientes asientos juntos disponibles en la sala." << '\n';
    }
    else
    {
        for (unsigned int i = 0; i < s.asientos_cliente; i++)
        {
            sala.at(reserva_inicial + i) = s.id_cliente;
        }
        std::cout << "Reserva del cliente " << s.id_cliente << " completada." << '\n';
    }
}

void solicitarEntradas(cliente c)
{
    solicitud s;
    s.id_cliente = c.id_cliente;
    s.asientos_cliente = c.numero_entradas;
    sem_aux.lock();
    v_solicitudes.insert(v_solicitudes.begin(), s);
    sem_aux.unlock();
    sem_taquilla.unlock();
}

void taquillaEntradas()
{
    while (1)
    {
        sem_taquilla.lock();
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        reservarHueco();
    }
}

void sistemaPago()
{
    while (1)
    {
        sem_pago.lock();
        realizarPago();
    }
}

void secuenciaCliente(int id){
    cliente c = {id,(rand() % 6) + 1,(rand() % 3) + 1,(rand() % 2) + 1};
    solicitarEntradas(c);
    solicitarPago(c.id_cliente, "entradas");

}

void creaHilos(int numero_hilos)
{
    for (unsigned int i = 1; i <= numero_hilos; i++)
    {
        v_entradas.push_back(std::thread(secuenciaCliente,i));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main(int argc, char const *argv[])
{
    vaciarSala();
    sem_taquilla.lock();
    std::thread taquilla(taquillaEntradas);

    sem_pago.lock();
    std::thread pagos(sistemaPago);

    creaHilos(5);
    //mostrarEstadoSala();

    pagos.join();

    return 0;
}
