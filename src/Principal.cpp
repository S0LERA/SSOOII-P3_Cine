#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>

std::vector<int> sala;

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

void reservarHueco(int id_hilo, int numero_asientos)
{
    int reserva_inicial = -1;
    reserva_inicial = comprobarReserva(numero_asientos);
    if (reserva_inicial == -1)
    {
        std::cout << "No hay suficientes asientos juntos disponibles en la sala." << '\n';
    }
    else
    {
        for (unsigned int i = 0; i < numero_asientos; i++)
        {
            sala.at(reserva_inicial + i) = id_hilo;
        }

        std::cout << "Reserva del cliente " << id_hilo << " completada." << '\n';
    }
}

void mostrarEstadoSala()
{
    for (unsigned int i = 0; i < 72; i++)
    {
        std::cout << "Asiento: " << i + 1 << "Reserva: " << sala.at(i) << "." << '\n';
    }
}
/*
void creaHilo(int id_hilo, int numero_asientos)
{
    std::thread hilo(reservarHueco, id_hilo, numero_asientos);
    hilo.join();
}
*/
int main(int argc, char const *argv[])
{
    vaciarSala();
    reservarHueco(3, 5);
    reservarHueco(2, 7);
    mostrarEstadoSala();
    return 0;
}
