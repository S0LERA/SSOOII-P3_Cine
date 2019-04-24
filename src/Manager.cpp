#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include <chrono>
#include <unistd.h>
#include "Structs.cpp"

std::vector<int> sala;
std::vector<std::thread> v_entradas;
std::vector<std::thread> cola_comida;
std::vector<solicitud> v_solicitudes;
std::vector<pago> v_pagos;
std::mutex sem_pago;
std::mutex sem_reserva;
std::mutex sem_taquilla;
std::mutex sem_aux;

void realizarPago(int id_cliente, std::string tipo)
{
    sem_pago.lock();
    //usleep(3000000);
    std::cout << "Cliente: " << id_cliente << " realiza el pago de " << tipo << "." << '\n';
    sem_pago.unlock();
}

/*
void moverCliente(int id)
{
    cola_comida.push_back(std::move(v_entradas.at(id)));
}
*/

void vaciarSala()
{
    for (unsigned int i = 0; i < 72; i++)
    {
        sala.push_back(-1);
    }
}

int comprobarReserva(int reserva)
{
    int contador = 0;
    for (unsigned int i = 0; i < 72; i++)
    {
        if (sala.at(i) == -1)
        {
            contador++;
            if (contador >= reserva)
            {
                return i + 1 - reserva;
            }
        }
        else
        {
            contador = 0;
        }
    }
    return -1;
}

void reservarHueco()
{
    int reserva_inicial = -1;

    solicitud s = std::move(v_solicitudes.front());
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
        realizarPago(s.id_cliente, "entradas");
        //moverCliente(id_hilo);
        //std::cout << "Reserva del cliente " << id_hilo << " completada." << '\n';
    }
}

void mostrarEstadoSala()
{
    for (unsigned int i = 0; i < 72; i++)
    {
        std::cout << "Asiento: " << i + 1 << " Reserva: " << sala.at(i) << "." << '\n';
    }
}

/*
void creaHilo(int id_hilo, int numero_asientos)
{
    std::thread hilo(reservarHueco, id_hilo, numero_asientos);
    hilo.join();
}
*/

void solicitarEntradas(int id_hilo, int numero_entradas)
{
    solicitud s;
    s.id_cliente = id_hilo;
    s.asientos_cliente = numero_entradas;
    sem_aux.lock();
    v_solicitudes.insert(v_solicitudes.begin(),s);
    sem_aux.unlock();
    sem_taquilla.unlock();
}

void creaHilos(int n, int a)
{
    
    for (unsigned int i = 1; i <= n; i++)
    {
        v_entradas.push_back(std::thread(solicitarEntradas, i, a));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        /*
        std::thread hilo(solicitarEntradas,i,a);
        hilo.join();
        */
    }
    for_each(v_entradas.begin(), v_entradas.end(), std::mem_fn(&std::thread::join));
}

void taquillaEntradas()
{
    while (1)
    {
        sem_taquilla.lock();
        reservarHueco();
    }
}

int main(int argc, char const *argv[])
{
    vaciarSala();
    sem_taquilla.lock();
    std::thread taquilla(taquillaEntradas);

    creaHilos(5, 5);
    mostrarEstadoSala();
        taquilla.detach();
    return 0;
}
