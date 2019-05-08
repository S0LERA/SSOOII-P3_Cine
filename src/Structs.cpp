struct solicitud{
    int id_cliente;
    int asientos_cliente;
}; 
struct pago{
    int id_cliente;
    std::string tipo_pago;
};
struct comida{
    int id_cliente;
    int palomitas;
    int bebidas;
};
struct punto_venta{
    int id;
    int palomitas;
    int bebidas;
};
struct cliente{
    int id_cliente;
    int numero_entradas;
    int palomitas;
    int bebidas;
};
