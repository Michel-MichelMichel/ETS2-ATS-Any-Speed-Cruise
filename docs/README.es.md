<p align="center">
  <img src="../assets/logo.png" width="180" alt="Logo de Any Speed Cruise">
</p>

# ETS2 & ATS – Any Speed Cruise

[English](../README.md) · [Français](README.fr.md) · [Deutsch](README.de.md) · **Español** · [Polski](README.pl.md) · [Čeština](README.cs.md)

Un control de crucero que funciona por debajo de 30 km/h (20 mph) en
**Euro Truck Simulator 2** y **American Truck Simulator**.

## Qué hace

En ambos juegos, «fijar» y «reanudar» no hacen nada por debajo de 30 km/h. Es
realista, pero los camiones tardan en acelerar. Con este plugin, tus teclas
habituales del control de crucero funcionan a cualquier velocidad:

| Tecla | Por debajo del límite del juego | Por encima |
| --- | --- | --- |
| Control de crucero (fijar) | El plugin mantiene la velocidad actual con el acelerador | El juego actúa como siempre |
| Reanudar | El plugin acelera hasta la última velocidad guardada, incluso desde parado | igual |
| Aumentar / reducir | Cambia el objetivo del plugin (5 km/h por defecto) | igual |

En cuanto el camión va lo bastante rápido, el plugin activa el control de
crucero real del juego y se retira. Suelta el acelerador en cuanto frenas,
vuelves a pulsar la tecla del control de crucero, pones el freno de mano,
metes la marcha atrás, apagas el motor o abres un menú.

## Por qué un plugin y no un mod

El límite de 30 km/h está escrito en el ejecutable del juego; un mod `.scs` no
puede cambiarlo. Este plugin solo usa el SDK oficial de SCS y no modifica ni
la memoria ni los archivos del juego.

## Instalación

1. Cierra el juego.
2. Descarga el último zip desde [Releases](../../../releases/latest).
3. Copia `any_speed_cruise.dll`, `any_speed_cruise.ini` y
   `any_speed_cruise_config.exe` en la carpeta `bin\win_x64\plugins` del juego
   (crea `plugins` si no existe).
4. Ejecuta `any_speed_cruise_config.exe` para indicar tus teclas.
5. Inicia el juego y acepta el aviso sobre funciones del SDK (es normal con
   cualquier plugin).

Para desinstalarlo, borra los tres archivos.

## Elegir las teclas

En `any_speed_cruise_config.exe`, haz clic en **Change** y pulsa una tecla o
un botón del volante / mando. El valor se guarda al instante (Esc cancela) y
el juego lo lee al arrancar.

El plugin solo observa esas teclas: **la misma tecla debe estar asignada a la
misma función en el juego** (Opciones > Teclas y botones).

Por defecto: teclado numérico `.`, `*`, `+` y `-`, que solo se reconocen con
Bloq Num activado.

## Conviene saber

- El testigo del control de crucero permanece apagado mientras el plugin
  mantiene la velocidad.
- El plugin solo usa el acelerador: no frena cuesta abajo.
- «Fijar» no hace nada con el camión parado; «reanudar» sí.
- Máximo 32 botones por dispositivo; sin crucetas (POV) ni botones del ratón.
- Probado en ETS2 1.62. Diseñado para ATS, pero **aún no probado** en él.

Solución de problemas, compilación y mantenimiento: consulta el
[README en inglés](../README.md).

## Creado por una IA

Todo el proyecto – plugin, herramienta, logo, documentación y traducciones –
fue creado por **Claude Fable 5.2** (Anthropic) en noviembre de 2026, a
petición de un jugador que lo probó en el juego. Úsalo bajo tu propia
responsabilidad. Licencia [MIT](../LICENSE). No afiliado a SCS Software.
