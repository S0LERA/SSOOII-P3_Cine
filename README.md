# SSOOII-P3_Cine
Se desea implementar un sistema de gestión para un cine que permita la compra de entradas para una sala y las palomitas de cada cliente.
 - Cada cliente se representará con un hilo y las taquillas para entradas y comida serán diferentes, habiendo 1 para entradas y 3 para comida.
 - El pago se realizará sobre un único recurso que deberá ser protegido de exclusión mutua.
 - Además habrá otro hilo encargado de reponer los recursos de las taquillas de comida cuando se acaben.

Se deberá llevar a cabo la gestión de la concurrencia de los hilos así como un sistema de prioridades para los mismos.

## Versiones
### Versión 0.5
 - Código empezado en versión secuencial:
   - Métodos para comprobar sala y hacer reservas listos.
  
### Versión 0.8
 - Código aactualizado para seguir el patrón cliente-servidor.
