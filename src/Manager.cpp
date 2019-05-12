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

#define C_CYAN "\033[1;36m"
#define C_AMARILLO "\033[1;33m"
#define C_VERDE "\033[1;32m"
#define C_FINAL "\033[0m\n"
#define C_ROJO "\033[1;31m"
#define C_MAGENTA "\033[1;35m"

std::vector<std::thread> v_hilos;
std::vector<comida> v_comida;
std::vector<solicitud> v_solicitudes;
std::vector<pago> v_pagos;
std::vector<punto_venta> v_reposiciones;
std::vector<int> v_abandonos;
std::mutex sem_pago;
std::mutex sem_taquilla;
std::mutex sem_comida;
std::mutex sem_reponer;
std::mutex sem_cola_pagos;
std::mutex sem_cola_reposiciones;
std::mutex sem_turnos_entradas;
std::mutex sem_turnos_taquilla;
std::mutex sem_turnos_comida;
std::mutex sem_turnos_puntov;
std::condition_variable cv_turno_entradas;
std::condition_variable cv_turno_comida;
std::condition_variable cv_puntos_venta;
int turno_entradas = 0;
int turno_comida = 0;
bool abandono = false;

void solicitarReposicion(punto_venta pv)
{
    sem_cola_reposiciones.lock();
    v_reposiciones.insert(v_reposiciones.begin(), pv);
    sem_cola_reposiciones.unlock();
    sem_reponer.unlock();
}

void solicitarEntradas(cliente c)
{
    const int id = c.id_cliente;
    std::unique_lock<std::mutex> lk_sem_turnos_taquilla(sem_turnos_taquilla);
    cv_turno_entradas.wait(lk_sem_turnos_taquilla, [id] { return (turno_entradas == id); });
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
    sem_cola_pagos.lock();
    v_pagos.insert(v_pagos.begin(), p);
    sem_cola_pagos.unlock();
    sem_pago.unlock();
}

void solicitarComida(cliente c)
{
    const int id = c.id_cliente;
    std::unique_lock<std::mutex> lk_sem_turnos_puntov(sem_turnos_puntov);
    cv_turno_comida.wait(lk_sem_turnos_puntov, [id] { return (turno_comida == id); });
    comida cm = {c.id_cliente, c.palomitas, c.bebidas};
    v_comida.insert(v_comida.begin(), cm);
    sem_comida.unlock();
    cv_puntos_venta.notify_one();
    sem_turnos_puntov.lock();
}

void taquillaEntradas()
{
    while (1)
    {
        sem_taquilla.lock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        int reserva_inicial = -1;

        solicitud s = v_solicitudes.front();
        v_solicitudes.erase(v_solicitudes.begin());
        reserva_inicial = comprobarReserva(s.asientos_cliente);

        std::cout << C_VERDE << "Cliente " << s.id_cliente << " quiere comprar " << s.asientos_cliente << " entradas." << C_FINAL << '\n';

        if (reserva_inicial == -1)
        {
            std::cout << C_ROJO << "No hay suficientes asientos juntos disponibles en la sala." << C_FINAL << '\n';
            v_abandonos.push_back(s.id_cliente);
            sem_turnos_taquilla.unlock();
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
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        if (p.tipo_pago == "entradas")
        {
            std::cout << C_VERDE << "Reserva del cliente " << p.id_cliente << " completada." << C_FINAL << '\n';
            sem_turnos_entradas.unlock();
        }
        else
        {
            std::cout << C_AMARILLO << "Pago de comida del cliente " << p.id_cliente << " completada." << C_FINAL << '\n';
            sem_turnos_comida.unlock();
        }
    }
}

void puntoVenta(int id, int palomitas, int bebidas)
{
    punto_venta pv = {id, palomitas, bebidas};
    while (1)
    {
        std::unique_lock<std::mutex> lk_sem_comida(sem_comida);
        cv_puntos_venta.wait(lk_sem_comida, [] { return !v_comida.empty(); });
        comida cm = v_comida.front();
        v_comida.erase(v_comida.begin());

        std::cout << C_AMARILLO << "Cliente " << cm.id_cliente << " Pide " << cm.palomitas << " palomitas y " << cm.bebidas << " bebidas." << C_FINAL << '\n';

        if (pv.palomitas - cm.palomitas < 0 || pv.bebidas - cm.bebidas < 0)
        {
            //std::cout << "Palomitas: "<< pv.palomitas << " Bebidas: " << pv.bebidas <<'\n';
            solicitarReposicion(pv);
            sem_comida.lock();
            pv = v_reposiciones.front();
            v_reposiciones.erase(v_reposiciones.begin());
            std::cout << C_MAGENTA << "Reposición del punto de venta " << pv.id << "." << C_FINAL << '\n';
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
    cliente c = {id, (rand() % 40) + 1, (rand() % 5) + 1, (rand() % 4) + 1};
    solicitarEntradas(c);
    solicitarPago(c, "entradas");
    solicitarComida(c);
    solicitarPago(c, "comida");
}

void creaHilos(int numero_hilos)
{
    for (unsigned int i = 1; i <= numero_hilos; i++)
    {
        v_hilos.push_back(std::thread(secuenciaCliente, i));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << C_CYAN << "Cliente: " << i << " llega al cine." << C_FINAL << '\n';
    }
}

int main(int argc, char const *argv[])
{

    if (argc > 1)
    {
        std::cout << C_ROJO << "Número de argumentos inválido, cerrando el programa..." << C_FINAL << '\n';
        return EXIT_FAILURE;
    }

    int n_hilos = 10;
    vaciarSala();
    sem_taquilla.lock();
    std::thread taquilla(taquillaEntradas);

    sem_pago.lock();
    std::thread pagos(sistemaPago);

    sem_comida.lock();
    std::thread punto_venta_1(puntoVenta, 1, 5, 5);
    std::thread punto_venta_2(puntoVenta, 2, 5, 5);
    std::thread punto_venta_3(puntoVenta, 3, 5, 5);

    sem_reponer.lock();
    std::thread reponedor_comida(reponedor);

    creaHilos(n_hilos);

    int aleatorio = 0;
    int ct_entradas = 0;
    int ct_comida = 0;
    sem_turnos_entradas.lock();
    sem_turnos_comida.lock();
    bool primera = true;
    while (1)
    {
        srand(time(0));
        aleatorio = (rand() % 4) + 1;
        if (aleatorio == 2 && primera == false && ct_entradas > ct_comida)
        {
            ct_comida++;
            turno_comida++;
            cv_turno_comida.notify_all();
            sem_turnos_comida.lock();
        }
        else if (ct_entradas < n_hilos)
        {
            primera = false;
            ct_entradas++;
            turno_entradas++;
            cv_turno_entradas.notify_all();
            sem_turnos_entradas.lock();
        }

        if (ct_comida == n_hilos)
        {
            mostrarEstadoSala();
            //return EXIT_SUCCESS; //Si dejamos esto el programa termina con una excepción
        }
    }
    reponedor_comida.join();

    return 0;
}
