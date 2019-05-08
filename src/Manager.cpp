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
std::vector<comida> v_comida;
std::vector<solicitud> v_solicitudes;
std::vector<pago> v_pagos;
std::vector<punto_venta> v_reposiciones;
std::mutex sem_pago;
std::mutex sem_reserva;
std::mutex sem_taquilla;
std::mutex sem_comida;
std::mutex sem_reponer;
std::mutex sem_aux;
std::mutex sem_aux2;
std::mutex sem_aux3;
std::mutex sem_aux4;

void realizarPago()
{
    pago p = v_pagos.front();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Cliente: " << p.id_cliente << " realiza el pago de " << p.tipo_pago << "." << '\n';
    v_pagos.erase(v_pagos.begin());
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

void solicitarReposicion(punto_venta pv)
{
    sem_aux4.lock();
    v_reposiciones.insert(v_reposiciones.begin(), pv);
    sem_aux3.unlock();
    sem_reponer.unlock();
}

void servirComida(punto_venta pv)
{
    comida cm = v_comida.front();
    v_comida.erase(v_comida.begin());

    if (pv.palomitas - cm.palomitas > 0)
    {
        solicitarReposicion(pv);
        pv = v_reposiciones.front();
        v_reposiciones.erase(v_reposiciones.begin());
        pv.palomitas = pv.palomitas - cm.palomitas;
    }
    else
    {
        pv.palomitas = pv.palomitas - cm.palomitas;
    }

    if (pv.bebidas - cm.bebidas > 0)
    {
        solicitarReposicion(pv);
        pv = v_reposiciones.front();
        v_reposiciones.erase(v_reposiciones.begin());
        pv.bebidas = pv.bebidas - cm.bebidas;
    }
    else
    {
        pv.bebidas = pv.bebidas - cm.bebidas;
    }

    std::cout << "AQUISs" << '\n';
}

void reponer()
{
    v_reposiciones.front().bebidas = 10;
    v_reposiciones.front().palomitas = 10;
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

void solicitarPago(cliente c, std::string tipo)
{
    pago p;
    p.id_cliente = c.id_cliente;
    p.tipo_pago = tipo;
    sem_aux2.lock();
    v_pagos.insert(v_pagos.begin(), p);
    sem_aux2.unlock();
    sem_pago.unlock();
}

void solicitarComida(cliente c)
{
    comida cm = {c.id_cliente, c.palomitas, c.bebidas};
    sem_aux3.lock();
    v_comida.insert(v_comida.begin(), cm);
    sem_aux3.unlock();
    sem_comida.unlock();
}

void taquillaEntradas()
{
    while (1)
    {
        sem_taquilla.lock();
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));aux
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

void puntoVenta(int palomitas, int bebidas)
{
    punto_venta pv = {palomitas, bebidas};
    while (1)
    {
        sem_comida.lock();
        comida cm = v_comida.front();
        v_comida.erase(v_comida.begin());

        std::cout << "Hilo " << cm.id_cliente << " Pide " << cm.palomitas << " palomitas y pide " << cm.bebidas << " bebidas." <<'\n';

        if (pv.palomitas - cm.palomitas < 0 || pv.bebidas - cm.bebidas < 0)
        {
            std::cout << "Palomitas: "<< pv.palomitas << " Bebidas: " << pv.bebidas <<'\n';
            solicitarReposicion(pv);
            sem_comida.lock();
            pv = v_reposiciones.front();
            v_reposiciones.erase(v_reposiciones.begin());
            std::cout << "Palomitas: "<< pv.palomitas << " Bebidas: " << pv.bebidas <<'\n';
            pv.palomitas = pv.palomitas - cm.palomitas;
            pv.bebidas = pv.bebidas - cm.bebidas;
        }
        else
        {

            std::cout << "Palomitas: "<< pv.palomitas << " Bebidas: " << pv.bebidas <<'\n';
            pv.palomitas = pv.palomitas - cm.palomitas;
            pv.bebidas = pv.bebidas - cm.bebidas;
        }

    }
}

void reponedor()
{
    while (1)
    {
        sem_reponer.lock();
        reponer();
        sem_comida.unlock();
    }
}

void secuenciaCliente(int id)
{
    cliente c = {id, (rand() % 6) + 1, (rand() % 3) + 1, (rand() % 2) + 1};
    solicitarEntradas(c);
    solicitarPago(c, "entradas");
    solicitarComida(c);
    solicitarPago(c, "comida");
}

void creaHilos(int numero_hilos)
{
    for (unsigned int i = 1; i <= numero_hilos; i++)
    {
        v_entradas.push_back(std::thread(secuenciaCliente, i));
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

    sem_comida.lock();
    std::thread comida(puntoVenta, 5, 5);

    sem_reponer.lock();
    std::thread reponedor_comida(reponedor);

    creaHilos(5);
    //mostrarEstadoSala();

    pagos.join();

    return 0;
}
