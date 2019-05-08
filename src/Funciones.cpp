std::vector<int> sala;

void mostrarEstadoSala()
{
    for (unsigned int i = 0; i < 72; i++)
    {
        std::cout << "Asiento: " << i + 1 << " Reserva: " << sala.at(i) << "." << '\n';
    }
}

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
