/**
 * Archivo: ArbolBPlus.cpp
 * Propósito: Aquí se implementan los métodos declarados en ArbolBPlus.h. 
 *            Esta es la clase central que los estudiantes deberán completar 
 *            como parte de su reto de Estructuras de Datos.
 */
#include "ArbolBPlus.h"

// -----------------------------------------------------------------------------
// Implementaciones de Estructuras Auxiliares
// -----------------------------------------------------------------------------

string Registro::serializar() const {
    // Ejemplo de salida: "1,Juan Perez" (clave + delimitador + datos)
    return to_string(clave) + "," + datos;
}

NodoBPlus::NodoBPlus(bool hoja) {
    // Por defecto al nacer, sabemos si es hoja o interno, pero no tiene nodos adyacentes aún.
    es_hoja = hoja;
    siguiente_hoja = nullptr;
}

// -----------------------------------------------------------------------------
// Constructor del Arbol B+
// -----------------------------------------------------------------------------

ArbolBPlus::ArbolBPlus(int _grado, string _nombre_archivo) : raiz(nullptr), grado(_grado), nombre_archivo(_nombre_archivo) {}

// =========================================================================
// MÉTODOS A IMPLEMENTAR PARA EL PARCIAL
// =========================================================================

void ArbolBPlus::insertar(int clave, string datos) {
    // [A IMPLEMENTAR EN EL PARCIAL]:
    // Lógica requerida:
    // 1. Si el árbol está vacío (raiz == nullptr), crear el primer nodo hoja.
    // 2. Si no está vacío, recorrer el árbol desde la raíz bajando por los hijos 
    //    correctos comparando la clave, hasta llegar a una hoja.
    // 3. Insertar el 'Registro' en el vector de registros de la hoja, MANTENIENDO EL ORDEN.
    // 4. Verificar condición de llenado: Si la hoja ahora tiene más elementos que el grado 
    //    (se desbordó), se debe dividir (SPLIT).
    // 5. El Split implica:
    //    a) Crear una nueva hoja.
    //    b) Pasar la mitad de los registros a la nueva hoja.
    //    c) Promover la clave media hacia el nodo PADRE.
    //    d) Configurar el puntero "siguiente_hoja" para mantener la lista enlazada unida.
    // 6. Esta propagación puede subir recursivamente hasta la raíz, obligando a crear una nueva raíz si es necesario.
    
    NodoBPlus* current_node = raiz;
    NodoBPlus* parent_node = nullptr;
    NodoBPlus* new_leaf = nullptr;
    NodoBPlus* new_root = nullptr;
    Registro new_record;
    int num_keys;
    int num_records;
    int mid_index;
    int promoted_key;
    int i;
    bool is_inserted = false;

    new_record.clave = clave;
    new_record.datos = datos;

    if (current_node == nullptr) {
        raiz = new NodoBPlus(true);
        raiz->registros.push_back(new_record);
        return;
    }

    while (current_node->es_hoja == false) {
        parent_node = current_node;
        num_keys = current_node->claves.size();
        for (i = 0; i < num_keys; i++) {
            if (clave < current_node->claves[i]) {
                break;
            }
        }
        current_node = current_node->hijos[i];
    }

    num_records = current_node->registros.size();
    for (i = 0; i < num_records; i++) {
        if (clave < current_node->registros[i].clave) {
            current_node->registros.insert(current_node->registros.begin() + i, new_record);
            is_inserted = true;
            break;
        }
    }
    
    if (is_inserted == false) {
        current_node->registros.push_back(new_record);
    }

    if ((int)current_node->registros.size() > grado) {
        new_leaf = new NodoBPlus(true);
        mid_index = current_node->registros.size() / 2;

        for (i = mid_index; i < (int)current_node->registros.size(); i++) {
            new_leaf->registros.push_back(current_node->registros[i]);
        }
        current_node->registros.erase(current_node->registros.begin() + mid_index, current_node->registros.end());

        new_leaf->siguiente_hoja = current_node->siguiente_hoja;
        current_node->siguiente_hoja = new_leaf;

        promoted_key = new_leaf->registros[0].clave;

        if (current_node == raiz) {
            new_root = new NodoBPlus(false);
            new_root->claves.push_back(promoted_key);
            new_root->hijos.push_back(current_node);
            new_root->hijos.push_back(new_leaf);
            raiz = new_root;
        } else {
            insertarInterno(promoted_key, parent_node, new_leaf);
        }
    }
}

string ArbolBPlus::buscar(int clave) {
    // [A IMPLEMENTAR EN EL PARCIAL]:
    // Lógica requerida:
    // 1. Si la raíz es nullptr, devolver string vacío (no hay datos).
    // 2. Empezar en la raíz y hacer una búsqueda binaria o lineal sobre 'claves'.
    // 3. Si la clave buscada es menor que claves[i], bajar por hijos[i].
    // 4. Si la clave es mayor o igual, seguir iterando o bajar por el último hijo.
    // 5. Al llegar a un nodo hoja (`es_hoja == true`), buscar el registro exacto.
    // 6. Si se encuentra, retornar `registro.datos`, de lo contrario retornar string vacío.
    
    NodoBPlus* current_node = raiz;
    int num_keys;
    int num_records;
    int i;
    
    if (current_node == nullptr) {
        return "";
    }
    
    while (current_node->es_hoja == false) {
        num_keys = current_node->claves.size();
        for (i = 0; i < num_keys; i++) {
            if (clave < current_node->claves[i]) {
                break;
            }
        }
        current_node = current_node->hijos[i];
    }
    
    num_records = current_node->registros.size();
    for (i = 0; i < num_records; i++) {
        if (current_node->registros[i].clave == clave) {
            return current_node->registros[i].datos;
        }
    }
    
    return "";
}

void ArbolBPlus::eliminar(int clave) {
    // [A IMPLEMENTAR EN EL PARCIAL]:
    // Lógica requerida:
    // 1. Localizar la hoja donde reside la clave.
    // 2. Eliminar el registro del vector.
    // 3. Verificar condición de 'underflow' (menos registros de los requeridos por el grado).
    // 4. Si hay underflow, intentar pedir prestado un registro a un nodo hermano (redistribución).
    // 5. Si no se puede pedir prestado, hacer 'merge' (fusión) con el hermano, 
    //    y eliminar la clave divisora en el nodo padre.
    
    NodoBPlus* current_node = raiz;
    int num_keys;
    int num_records;
    int min_records;
    int i;
    bool is_found = false;

    if (current_node == nullptr) {
        return;
    }

    while (current_node->es_hoja == false) {
        num_keys = current_node->claves.size();
        for (i = 0; i < num_keys; i++) {
            if (clave < current_node->claves[i]) {
                break;
            }
        }
        current_node = current_node->hijos[i];
    }

    num_records = current_node->registros.size();
    for (i = 0; i < num_records; i++) {
        if (current_node->registros[i].clave == clave) {
            current_node->registros.erase(current_node->registros.begin() + i);
            is_found = true;
            break;
        }
    }

    if (is_found == false) {
        return;
    }

    min_records = grado / 2;
    if (current_node != raiz && (int)current_node->registros.size() < min_records) {
        cout << "[Underflow] Paila, la hoja quedo muy vacia. Toca fusionar hermanos.\n";
    }
}

vector<Registro> ArbolBPlus::obtenerTodos() {
    // [A IMPLEMENTAR EN EL PARCIAL]:
    // Lógica requerida:
    // 1. Bajar desde la raíz usando siempre el hijo[0] hasta llegar a la primera hoja (la más a la izquierda).
    // 2. Recorrer los registros de esa hoja e insertarlos en 'resultado'.
    // 3. Usar el puntero 'siguiente_hoja' para saltar a la próxima hoja.
    // 4. Repetir hasta que 'siguiente_hoja' sea nullptr.
    // Esto simula un comportamiento O(n) extremadamente rápido típico de las bases de datos (Full Table Scan).
    
    vector<Registro> result_list;
    NodoBPlus* current_node;
    int i;
    int num_records;

    current_node = raiz;

    if (current_node == nullptr) {
        return result_list;
    }

    while (current_node->es_hoja == false) {
        current_node = current_node->hijos[0];
    }

    while (current_node != nullptr) {
        num_records = current_node->registros.size();
        for (i = 0; i < num_records; i++) {
            result_list.push_back(current_node->registros[i]);
        }
        current_node = current_node->siguiente_hoja;
    }

    return result_list;
}

// =========================================================================
// MÉTODOS DE PERSISTENCIA (ARCHIVO DE TEXTO)
// =========================================================================

void ArbolBPlus::guardarEnArchivo() {
    ofstream output_file(nombre_archivo);
    vector<Registro> all_records;
    int i;
    int total_records;
    
    // Si no tenemos permisos o la ruta falla, abortamos
    if (!output_file.is_open()) {
        cerr << "Error al abrir el archivo " << nombre_archivo << endl;
        return;
    }
    
    // [A IMPLEMENTAR EN EL PARCIAL]:
    // 1. Invocar 'obtenerTodos()' o hacer el recorrido manual de hojas.
    // 2. Por cada registro obtenido, llamar a 'registro.serializar()' y escribir esa cadena en el archivo.
    // 3. Añadir un salto de línea (endl) por cada registro.
    
    all_records = obtenerTodos();
    total_records = all_records.size();

    for (i = 0; i < total_records; i++) {
        output_file << all_records[i].serializar() << "\n";
    }

    output_file.close();
}

void ArbolBPlus::cargarDesdeArchivo() {
    ifstream input_file(nombre_archivo);
    string current_line;
    string id_str;
    string data_str;
    int comma_pos;
    int record_id;
    
    // Si el archivo no existe (ej. es la primera vez que corre el programa), ignorar sin error grave.
    if (!input_file.is_open()) {
        cout << "No existe archivo previo '" << nombre_archivo << "'. Se creará al guardar.\n";
        return;
    }
    
    // [A IMPLEMENTAR EN EL PARCIAL]:
    // 1. Leer línea por línea usando `getline(archivo, linea)`.
    // 2. Partir/Separar (Split) el string basándose en la coma ','.
    // 3. Convertir la primera parte a entero (ID).
    // 4. Pasar la segunda parte como string (Datos).
    // 5. Llamar al método `insertar(id, datos)` del mismo árbol B+ para poblarlo en memoria RAM.
    
    while (getline(input_file, current_line)) {
        if (current_line.empty()) continue;

        comma_pos = current_line.find(',');
        if (comma_pos != string::npos) {
            id_str = current_line.substr(0, comma_pos);
            data_str = current_line.substr(comma_pos + 1);
            record_id = stoi(id_str);
            insertar(record_id, data_str);
        }
    }

    input_file.close();
}

// -----------------------------------------------------------------------------
// METODOS AUXILIARES (Para que el compilador no chille)
// -----------------------------------------------------------------------------
void ArbolBPlus::insertarInterno(int clave, NodoBPlus* cursor, NodoBPlus* hijo) {
    // Funcion de apoyo
}

NodoBPlus* ArbolBPlus::buscarPadre(NodoBPlus* cursor, NodoBPlus* hijo) {
    // Funcion de apoyo
    return nullptr;
}