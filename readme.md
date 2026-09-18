# Proyecto 2 : Motor de Base de Datos SQL (Árboles B+)

## 1. Contexto Académico

* **Institución:** Universidad EAFIT
* **Evaluación:** Parcial 2 - Estructuras de Datos y Algoritmos
  
* **Objetivo:** Diseñar e implementar desde cero un motor de base de datos relacional rudimentario, integrando un analizador léxico/sintáctico interactivo (Parser) con una estructura de datos jerárquica de Árbol B+ para el almacenamiento, búsqueda y persistencia de registros en memoria y disco.

## 2. Descripción General de la Arquitectura
El sistema está construido bajo un patrón de diseño de tres capas acopladas, garantizando la separación de responsabilidades y el manejo manual de la memoria mediante punteros, sin depender de estructuras asociativas estándar (como `std::map` o bases de datos preexistentes).

1.  **Capa de Interfaz (CLI / REPL):** Implementada en `main.cpp`. Actúa como el punto de entrada del usuario, manteniendo un bucle de ejecución continuo que captura las entradas de texto en bruto a través del prompt `sql> `. Se encarga de la inicialización segura del motor y de forzar el guardado de seguridad en disco al detectar el comando de salida.
2.  **Capa de Análisis (Parser SQL):** Implementada en `AnalizadorSQL.cpp`. Actúa como el traductor del sistema. Recibe la cadena de texto plana del usuario, normaliza la instrucción principal a mayúsculas y extrae mediante manipulación de *strings* (búsqueda de delimitadores como paréntesis, comas y comillas simples) la semántica de la consulta, los identificadores numéricos y las cadenas de caracteres.
3.  **Capa de Estructura de Datos (Árbol B+):** Implementada en `ArbolBPlus.cpp`. Es el núcleo del motor. Maneja un Árbol B+ de grado 3 configurado para almacenar claves enteras (IDs) asociadas a cadenas de texto. Administra la lógica matemática de división de nodos (*split*), la promoción de claves hacia la raíz dinámica y el enlace de nodos hoja para barridos secuenciales.
4.  **Capa de Persistencia:** Manejada transaccionalmente. El estado completo de los nodos del árbol en la memoria RAM se serializa y se guarda en un archivo secuencial plano denominado `base_datos.txt`. Al reiniciar el sistema, este módulo lee el archivo y reconstruye la topología del árbol B+ de manera íntegra.

## 3. Funcionalidades y Algoritmos Implementados

### 3.1. Inserción Ordenada y División de Nodos (INSERT)
El motor soporta la inserción estructurada de datos. Cuando el analizador extrae el ID y el dato de un comando `INSERT`, la estructura desciende algorítmicamente hasta la hoja correspondiente en tiempo O(log N). Si la hoja supera la capacidad máxima de claves (determinada por el grado del árbol), se ejecuta un algoritmo de *split* que divide la hoja en dos, redistribuye los registros, enlaza la nueva hoja en la lista enlazada inferior y promueve la clave central al nodo padre, ajustando la raíz dinámicamente si es necesario.

### 3.2. Búsquedas Logarítmicas y Secuenciales (SELECT)
*   **Búsqueda Logarítmica (Puntual):** Al detectar una cláusula `WHERE id = X`, el árbol realiza un recorrido descendente comparando las claves internas para encontrar la ruta exacta hacia la hoja que contiene el registro, logrando tiempos de respuesta óptimos.
*   **Recorrido Secuencial (Full Scan):** Al procesar un comando `SELECT * FROM tabla`, el árbol localiza la primera hoja (extremo izquierdo) y utiliza los punteros de siguiente nodo (*next*) para barrer e imprimir todos los registros en orden ascendente sin necesidad de recorrer los nodos internos nuevamente.

### 3.3. Eliminación Directa (DELETE)
El motor permite la búsqueda y eliminación de un registro específico en la estructura de memoria RAM. Al identificar el ID objetivo, se suprime el dato del nodo hoja. Actualmente, el sistema notifica alertas de *underflow* en la consola si la eliminación deja un nodo con menos registros del mínimo permitido, asegurando la integridad de los datos restantes aunque no realice un rebalanceo complejo.

### 3.4. Cascarones de Pantalleo (DDL)
Se integró el reconocimiento léxico para los comandos de Definición de Datos (`CREATE TABLE`, `CREATE INDEX`, `DROP TABLE`). Aunque el analizador procesa la sintaxis correctamente y notifica su ejecución, estas instrucciones operan funcionalmente como prototipos (*stubs*) informativos que no modifican la estructura del archivo plano principal, demostrando la capacidad del *parser* para extender su gramática en futuras versiones.

## 4. Diccionario de Comandos SQL Soportados

| Comando / Familia SQL | Sintaxis Estricta Requerida | Acción del Motor |
| :--- | :--- | :--- |
| **DML: Inserción** | `INSERT INTO tabla VALUES (id, 'dato')` | Extrae el entero y la cadena delimitada por comillas simples; balancea el árbol y almacena el registro. |
| **DQL: Búsqueda Puntual** | `SELECT * FROM tabla WHERE id = X` | Rastrea el identificador `X` en tiempo logarítmico e imprime su valor asociado. |
| **DQL: Barrido Total** | `SELECT * FROM tabla` | Ejecuta un escaneo horizontal de todas las hojas del árbol e imprime la base de datos completa. |
| **DML: Eliminación** | `DELETE FROM tabla WHERE id = X` | Suprime el registro `X` de la estructura en memoria e informa sobre el estado de la hoja. |
| **DDL: Creación / Índices** | `CREATE TABLE ...` / `CREATE INDEX ...` | Comandos reconocidos por el *parser* (simulación de ejecución estructural). |
| **Control: Salida Segura** | `EXIT` (o `exit`, `quit`) | Invoca la serialización, escribe los datos en disco y finaliza el proceso. |
| **Control: Manual** | `HELP` | Imprime el glosario de comandos interactivos con sintaxis de ejemplo resaltada. |

## 5. Instrucciones de Compilación y Ejecución

El proyecto incluye un archivo `Makefile` preconfigurado para compilar las tres clases estructurales (`main`, `ArbolBPlus`, `AnalizadorSQL`) utilizando el estándar `g++` nativo. 

Para desplegar y evaluar el motor en un entorno Linux/WSL, siga esta secuencia de comandos en la terminal de su preferencia:

1.  **Limpieza del entorno (Opcional pero recomendado):**
    Elimina los binarios preexistentes, los objetos de compilación `.o` y purga el archivo de persistencia anterior para realizar una evaluación limpia.
    ```bash
    make clean
    ```
2.  **Compilación del Motor:**
    Ensambla el código fuente garantizando el enlazamiento de los punteros del árbol B+ con el analizador léxico.
    ```bash
    make
    ```
3.  **Ejecución del Binario:**
    Inicia la instancia de la base de datos en memoria principal y habilita la consola de interacción (REPL).
    ```bash
    ./bd_sql
    ```

## 6. Persistencia y Almacenamiento en Disco
El sistema garantiza que la información no sea volátil. Toda interacción finalizada mediante el comando `EXIT` consolidará la información contenida en el Árbol B+ en un archivo local llamado `base_datos.txt`. La estructura de este archivo sigue un formato estricto de valores separados por comas (`clave,valor`), permitiendo que el constructor de la clase `ArbolBPlus` lea el archivo y reconstruya el entorno de trabajo exacto de la sesión anterior sin pérdida de integridad referencial.
