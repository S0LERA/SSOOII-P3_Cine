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
#include <condition_variable>

std::vector<std::thread> v_entradas;
std::vector<comida> v_comida;
std::vector<solicitud> v_solicitudes;
std::vector<pago> v_pagos;
std::vector<punto_venta> v_reposiciones;
std::mutex sem_pago;
std::mutex sem_taquilla;
std::mutex sem_comida;
std::mutex sem_reponer;
std::mutex sem_aux2;
std::mutex sem_aux3;
std::mutex sem_aux4;
std::mutex sem_turnos_entradas;
std::mutex sem_turnos_taquilla;
std::mutex sem_turnos_comida;
std::mutex sem_turnos_puntov;
std::condition_variable cv_turno_entradas;
std::condition_variable cv_turno_comida;
int turno_entradas = 0;
int turno_comida = 0;

void solicitarReposicion(punto_venta pv)
{
    sem_aux4.lock();
    v_reposiciones.insert(v_reposiciones.begin(), pv);
    sem_aux4.unlock();
    sem_reponer.unlock();
}

void solicitarEntradas(cliente c)
{
    const int id = c.id_cliente;
    std::unique_lock<std::mutex> lk_sem_turnos_taquilla(sem_turnos_taquilla);
    cv_turno_entradas.wait(lk_sem_turnos_taquilla,[id]{return(turno_entradas == id);});
    solicitud s;
    s.id_cliente = c.id_cliente;
    s.asientos_cliente = c.numero_entradas;
    v_solicitudes.insert(v_solicitudes.begin(), s);
    sem_taquilla.unlock();
    sem_turnos_taquilla.lock();
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
    const int id = c.id_cliente;
    std::unique_lock<std::mutex> lk_sem_turnos_puntov(sem_turnos_puntov);
    cv_turno_comida.wait(lk_sem_turnos_puntov,[id]{return(turno_comida == id);});
    comida cm = {c.id_cliente, c.palomitas, c.bebidas};
    sem_aux3.lock();
    v_comida.insert(v_comida.begin(), cm);
    sem_aux3.unlock();
    sem_comida.unlock();
    sem_turnos_puntov.lock();
}

void taquillaEntradas()
{
    while (1)
    {
        sem_taquilla.lock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        int reserva_inicial = -1;

        solicitud s = v_solicitudes.front();
        v_solicitudes.erase(v_solicitudes.begin());
        reserva_inicial = comprobarReserva(s.asientos_cliente);

        std::cout << "Cliente " << s.id_cliente << " quiere comprar " << s.asientos_cliente << " entradas." <<'\n';

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

            sem_turnos_taquilla.unlock();
        }
    }
}

void sistemaPago()
{
    while (1)
    {
        sem_pago.lock();
        pago p = v_pagos.front();
        v_pagos.erase(v_pagos.begin());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (p.tipo_pago == "entradas")
        {
            std::cout << "Reserva del cliente " << p.id_cliente << " completada." << '\n';
            sem_turnos_entradas.unlock();
        }
        else
        {
            std::cout << "Pago de comida del cliente " << p.id_cliente << " completada." << '\n';
            sem_turnos_comida.unlock();
        }
    }
}

void puntoVenta(int id, int palomitas, int bebidas)
{
    punto_venta pv = {id, palomitas, bebidas};
    while (1)
    {
        sem_comida.lock();
        comida cm = v_comida.front();
        v_comida.erase(v_comida.begin());

        std::cout << "Cliente " << cm.id_cliente << " Pide " << cm.palomitas << " palomitas y " << cm.bebidas << " bebidas." << '\n';

        if (pv.palomitas - cm.palomitas < 0 || pv.bebidas - cm.bebidas < 0)
        {
            //std::cout << "Palomitas: "<< pv.palomitas << " Bebidas: " << pv.bebidas <<'\n';
            solicitarReposicion(pv);
            sem_comida.lock();
            pv = v_reposiciones.front();
            v_reposiciones.erase(v_reposiciones.begin());
            std::cout << "Reposición del punto de venta " <<pv.id<<"."<< '\n';
            //std::cout << "Palomitas: "<< pv.palomitas << " Bebidas: " << pv.bebidas <<'\n';
            pv.palomitas = pv.palomitas - cm.palomitas;
            pv.bebidas = pv.bebidas - cm.bebidas;
            sem_turnos_puntov.unlock();
        }
        else
        {
            //std::cout << "Palomitas: "<< pv.palomitas << " Bebidas: " << pv.bebidas <<'\n';
            pv.palomitas = pv.palomitas - cm.palomitas;
            pv.bebidas = pv.bebidas - cm.bebidas;
            sem_turnos_puntov.unlock();
        }
    }
}

void reponedor()
{
    while (1)
    {
        sem_reponer.lock();
        v_reposiciones.front().bebidas = 10;
        v_reposiciones.front().palomitas = 10;
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
    int n_hilos = 10;
    vaciarSala();
    sem_taquilla.lock();
    std::thread taquilla(taquillaEntradas);

    sem_pago.lock();
    std::thread pagos(sistemaPago);

    sem_comida.lock();
    std::thread punto_venta_1(puntoVenta,1 , 5, 5);

    sem_reponer.lock();
    std::thread reponedor_comida(reponedor);

    creaHilos(n_hilos);
    //mostrarEstadoSala();

    sem_turnos_entradas.lock();
    for(int i = 0; i<n_hilos;i++){
        turno_entradas++;
        cv_turno_entradas.notify_all();
        sem_turnos_entradas.lock();
    }

    sem_turnos_comida.lock();
    for(int i = 0; i<n_hilos;i++){
        turno_comida++;
        cv_turno_comida.notify_all();
        sem_turnos_comida.lock();
    }

    reponedor_comida.join();

    return 0;
}
