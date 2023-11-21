#include <iostream>
#include <fstream>
#include <string>
#include <functional>

#define M 31

using namespace std;

template <typename K, typename V>
class MultiHashMap{
private:
    struct Node{
        K m_Key;
        V m_Value;
        Node* m_Next;
        Node(const K& k, const V& v) : m_Key(k), m_Value(v), m_Next(nullptr) {}
    };

    Node** m_Table;
    int m_Size; /// количество элементов
    int m_Capacity; ///Размер массива хэштаблицы
    double m_LoadFactor = 0.75;
    hash<K> Hasher;

public:
    MultiHashMap();


    void clear();
    ///Правило 3-х
    ~MultiHashMap();
    MultiHashMap(const MultiHashMap& other);
    MultiHashMap& operator=(const MultiHashMap& other);

    void Resize();///Функция для увеличения размера хеш-таблицы
    V ElementSeatch(const K& key);///Поиск элемента в коллекции по заданному ключу.
    void Insert(const K& key, const V& value); ///Добавление нового элемента с заданными ключом и значением в коллекцию.
    void remove(const K& key);/// Удаление из коллекции элемента с заданным ключом.
    void PrintMap();

    // Iterator class
    class Iterator {
    private:
        const MultiHashMap& m_Map;
        int m_Index;
        Node* m_Indicator;

    public:
        Iterator(const MultiHashMap& MultiHashMap);
        Iterator(const MultiHashMap& MultiHashMap, int Index, Node* Indicator);

        bool HasNext() const;
        void Next();///ищем следующий непустой бакет
        const K& getKey() const;
        const V& getValue() const;

        Iterator& operator++();

        void MoveToTheNext();
    };

    Iterator GetIterator() const;

    Iterator Begin() const;
    Iterator End() const; ///последний бакет

    int getTotalSize() const; ///вывод количества элементов в мапе

    int getUniqueSize() const; ///вывод количества уникальных элементов

    V* GetAllElements(const K& key) const;
    int GetNumberEl(const K& key) const;

};

template <typename K, typename V>
MultiHashMap<K, V>::MultiHashMap() : m_Size(0), m_Capacity(M) {
    m_Table = new Node*[m_Capacity]();
}

template <typename K, typename V>
void MultiHashMap<K, V>::clear() {
    for (int i = 0; i < m_Capacity; ++i) {
        Node* current = m_Table[i];
        while (current != nullptr) {
            Node* next = current->m_Next;
            delete current;
            current = next;
        }
        m_Table[i] = nullptr;
    }
    m_Size = 0;
}

template <typename K, typename V>
MultiHashMap<K, V>::MultiHashMap(const MultiHashMap& other){
    // Выделение памяти для новой таблицы
    m_Table = new Node*[m_Capacity];
    // Инициализация каждого элемента таблицы
    for (int i = 0; i < m_Capacity; ++i) {
        if (other.m_Table[i] != nullptr) {
            // Копирование элементов из существующей таблицы в новую
            m_Table[i] = new Node(other.m_Table[i]->m_Key, other.m_Table[i]->m_Value);
            Node* current = m_Table[i];
            Node* otherCurrent = other.m_Table[i]->m_Next;
            while (otherCurrent != nullptr) {
                // Копирование элементов из связанных списков
                current->m_Next = new Node(otherCurrent->m_Key, otherCurrent->m_Value);
                current = current->m_Next;
                otherCurrent = otherCurrent->m_Next;
            }
        } else {
            m_Table[i] = nullptr;
        }
    }
}

template <typename K, typename V>
MultiHashMap<K, V>::~MultiHashMap(){
    // Освобождение памяти при удалении объекта
    clear();
    delete[] m_Table;
}

template <typename K, typename V>
MultiHashMap<K, V>& MultiHashMap<K, V>::operator=(const MultiHashMap& other) {
    if (this != &other) {  // Check for self-assignment
        // Free resources of the current object
        clear();

        // Copy data from the other object
        m_Size = other.m_Size;
        m_Capacity = other.m_Capacity;
        m_LoadFactor = other.m_LoadFactor;

        // Allocate memory for the new table
        m_Table = new Node*[m_Capacity];

        // Copy elements from the other object
        for (int i = 0; i < m_Capacity; ++i) {
            if (other.m_Table[i] != nullptr) {
                m_Table[i] = new Node(other.m_Table[i]->m_Key, other.m_Table[i]->m_Value);
                Node* current = m_Table[i];
                Node* otherCurrent = other.m_Table[i]->m_Next;
                while (otherCurrent != nullptr) {
                    current->m_Next = new Node(otherCurrent->m_Key, otherCurrent->m_Value);
                    current = current->m_Next;
                    otherCurrent = otherCurrent->m_Next;
                }
            } else {
                m_Table[i] = nullptr;
            }
        }
    }
    return *this;
}

template <typename K, typename V>
void MultiHashMap<K, V>::Resize() {
    int newCapacity = m_Capacity * 2;
    Node** m_NewTable = new Node*[newCapacity];
    for (int i = 0; i < m_Capacity; ++i) {
        Node* m_Indicator = m_Table[i]; ///указателем на узел в хеш-таблице
        while (m_Indicator != nullptr) {
            Node* m_Next = m_Indicator->m_Next;
            int index = Hasher(m_Indicator->m_Key) % newCapacity;
            m_Indicator->m_Next = m_NewTable[index];
            m_NewTable[index] = m_Indicator;
            m_Indicator = m_Next;
        }
    }

    delete[] m_Table;
    m_Table = m_NewTable;
    m_Capacity = newCapacity;
}

template <typename K, typename V>
V MultiHashMap<K, V>::ElementSeatch(const K &key) {
    int index = Hasher(key)%m_Capacity;
    Node* Indicator = m_Table[index];

    while(Indicator != nullptr){
        if(Indicator->m_Key == key){
            return Indicator->m_Value;
        }
        Indicator = Indicator->m_Next;
    }
}

template <typename K, typename V>
void MultiHashMap<K, V>::Insert(const K &key, const V &value){ ///???Нужно ли m_Size преобразовать в double static_cast<double>
    if (m_Size / m_Capacity >= m_LoadFactor) {
        Resize();
    }

    int index = Hasher(key) % m_Capacity; ///гарантируем,что индекс остаеётся в пределах допустимых границ
    Node* m_Indicator = m_Table[index]; ///указатель на начало цепочки
    Node* prev = nullptr; ///указатель на предыдущий элемент в списке

    while (m_Indicator != nullptr) {
//        if (m_Indicator->m_Key == key) {///элемент с таким ключом key уже существует в хеш-таблице. => обновляем значение этого элемента
//            m_Indicator->m_Value = value;
//            return;
//        }
        ///Двигаем указатели
        prev = m_Indicator;
        m_Indicator = m_Indicator->m_Next;
    }

    /// вставляет новый элемент в хеш-таблицу
    Node* newNode = new Node(key, value);

    if (prev != nullptr) {
        prev->m_Next = newNode;
    } else {
        m_Table[index] = newNode;
    }
    ++m_Size;
}

template <typename K, typename V>
void MultiHashMap<K, V>::remove(const K &key)   { ///удаления элемента из хеш-таблицы
    int index = Hasher(key) % m_Capacity;
    Node* m_Indicator = m_Table[index];
    Node* prev = nullptr;

    while (m_Indicator != nullptr) {
        if (m_Indicator->m_Key == key) { ///Удаляем элемент
            if (prev != nullptr) {
                prev->m_Next = m_Indicator->m_Next;
            } else {///обновляет указатель на начало цепочки
                m_Table[index] = m_Indicator->m_Next;
            }
            delete m_Indicator;
            --m_Size;
//            return;
        }
        prev = m_Indicator;
        m_Indicator = m_Indicator->m_Next;
    }
}

template <typename K, typename V>
void MultiHashMap<K, V>::PrintMap() {
    Node* m_Indicator;
    for(int i = 0; i < m_Capacity; i++){
        cout << i << ": ";
        m_Indicator = m_Table[i];
        while(m_Indicator != nullptr){
            cout << m_Indicator->m_Value << " ";
            m_Indicator = m_Indicator->m_Next;
        }
        cout << endl;
    }

}

template<typename K, typename V>
typename MultiHashMap<K, V>::Iterator MultiHashMap<K, V>::Begin() const {
    int index = 0;
    while (index < m_Capacity && m_Table[index] == nullptr) {
        index++;
    }
    if (index < m_Capacity) {
        return Iterator(*this, index, m_Table[index]);
    } else {
        return End();
    }
} ///ищем первый не пустой бакет
template<typename K, typename V>

typename MultiHashMap<K, V>::Iterator MultiHashMap<K, V>::End() const {
    return Iterator(*this, m_Capacity, nullptr);
}

template <typename K, typename V>
int MultiHashMap<K, V>::getTotalSize() const {
    return m_Size;
}

template <typename K, typename V>
int MultiHashMap<K,V>::getUniqueSize() const {
    int count = 0;

    auto it = this->Begin();
    auto EndIt = this->End();
    V* ContainerValues = new V[m_Size];
    while (it.HasNext()) {
        V value =it.getValue();
        int flag = 1;
        for (int i = 0; i < count; ++i) {
            if(ContainerValues[i] == value){
                flag = 0;
            }
        }
        if(flag == 1){
            ContainerValues[count] = value;
            count++;
        }
        ++it;
    }
    delete[] ContainerValues;
    return count;
}

template<typename K, typename V>
MultiHashMap<K, V>::Iterator::Iterator(const MultiHashMap <K, V> &MultiHashMap): m_Map(MultiHashMap), m_Index(0), m_Indicator(nullptr) {
    MoveToTheNext();
}

template<typename K, typename V>
MultiHashMap<K, V>::Iterator::Iterator(const MultiHashMap <K, V> &MultiHashMap, int Index, Node *Indicator) : m_Map(MultiHashMap), m_Index(Index), m_Indicator(Indicator) {};

template<typename K, typename V>
bool MultiHashMap<K, V>::Iterator::HasNext() const {
    return m_Indicator != nullptr;
}

template<typename K, typename V>
void MultiHashMap<K, V>::Iterator::Next() {
    m_Indicator = m_Indicator->m_Next;
    if (!m_Indicator) {
        m_Index++;
        MoveToTheNext();
    }
}

template<typename K, typename V>
const K& MultiHashMap<K, V>::Iterator::getKey() const {
    return m_Indicator->m_Key;
}

template<typename K, typename V>
const V& MultiHashMap<K, V>::Iterator::getValue() const {
    return m_Indicator->m_Value;
}

template<typename K, typename V>
typename MultiHashMap<K, V>::Iterator& MultiHashMap<K, V>::Iterator::operator++() {
    Next();
    return *this;
}

template<typename K, typename V>
void MultiHashMap<K,V>::Iterator::MoveToTheNext() {
    while (m_Index < m_Map.m_Capacity && m_Map.m_Table[m_Index] == nullptr) {
        m_Index++;
    }

    if (m_Index < m_Map.m_Capacity) {
        m_Indicator = m_Map.m_Table[m_Index];
    } else {
        m_Indicator = nullptr;
    }
} ///ищем следующий непустой бакет

template<typename K, typename V>
typename MultiHashMap<K,V>::Iterator MultiHashMap<K,V>::GetIterator() const {
    return Iterator(*this);
}


template<typename K, typename V>
V* MultiHashMap<K,V>::GetAllElements(const K& key) const {
    int index = Hasher(key)%m_Capacity;
    Node* Indicator = m_Table[index];
    int cap = 31;
    V* ans = new V[cap];
    int cou = 1;

    while(Indicator != nullptr){
        if(Indicator->m_Key == key){
            ans[cou] = Indicator->m_Value;
            cou++;
            if(cou == cap){
                int newCap = cap * 2;
                V* Newans = new V[newCap];
                for (int i = 0; i < m_Capacity; ++i) {
                    Newans[i] = ans[i];
                }

                delete[] ans;
                ans = Newans;
                cap = newCap;
            }
        }
        Indicator = Indicator->m_Next;
    }
    if( cou == 0){
        return nullptr;
    }
    ans[0] = cou;
    return ans;
}

template<typename K, typename V>
int MultiHashMap<K,V>::GetNumberEl(const K &key) const {
    V* m_Arr = GetAllElements(key);
    return m_Arr[0]-1;
}



int main() {
    system("chcp 65001"); // Русификация ввода/вывода
    ifstream F("input.txt"); //открываем файл "input.txt"
    ofstream Out("output.txt"); //открываем файл "output.txt"

    if(!F && !Out){
        cout << "Не удалось открыть файл"<< endl;
        return 1;
    }

    char keyType, valueType, comand;
    F >> keyType >> valueType;
    int N;
    F >> N;

    switch (keyType) {
        case ('I'): {
            int key;
            switch (valueType) {
                case 'I':{
                    int value;
                    MultiHashMap<int, int> m_HashMap;
                    for (int i = 0; i < N; ++i) {
                        F >> comand;
                        if (comand == 'A') {
                            F >> key >> value;
                            m_HashMap.Insert(key, value);
                        } else {
                            F >> key;
                            m_HashMap.remove(key);
                        }
                    }
                    cout << m_HashMap.getTotalSize() << " " << m_HashMap.getUniqueSize();
                    break;}
                case 'D':{
                    double value;
                    MultiHashMap<int, double> m_HashMap;
                    for (int i = 0; i < N; ++i) {
                        F >> comand;
                        if (comand == 'A') {
                            F >> key >> value;
                            m_HashMap.Insert(key, value);
                        } else {
                            F >> key;
                            m_HashMap.remove(key);
                        }
                    }
                    cout << m_HashMap.getTotalSize() << " " << m_HashMap.getUniqueSize();
                    break;
                }
                case 'S': {
                    string value;
                    MultiHashMap<int, string> m_HashMap;
                    for (int i = 0; i < N; ++i) {
                        F >> comand;
                        if (comand == 'A') {
                            F >> key >> value;
                            m_HashMap.Insert(key, value);
                        } else {
                            F >> key;
                            m_HashMap.remove(key);
                        }
                    }
                    cout << m_HashMap.getTotalSize() << " " << m_HashMap.getUniqueSize();
                    break;
                }
            }
            break;
        }
        case ('D'): {
            double key;
            switch (valueType) {
                case 'I':{
                    int value;
                    MultiHashMap<double, int> m_HashMap;
                    for (int i = 0; i < N; ++i) {
                        F >> comand;
                        if (comand == 'A') {
                            F >> key >> value;
                            m_HashMap.Insert(key, value);
                        } else {
                            F >> key;
                            m_HashMap.remove(key);
                        }
                    }
                    cout << m_HashMap.getTotalSize() << " " << m_HashMap.getUniqueSize();
                    break;}
                case 'D':{
                    double value;
                    MultiHashMap<double, double> m_HashMap;
                    for (int i = 0; i < N; ++i) {
                        F >> comand;
                        if (comand == 'A') {
                            F >> key >> value;
                            m_HashMap.Insert(key, value);
                        } else {
                            F >> key;
                            m_HashMap.remove(key);
                        }
                    }
                    cout << m_HashMap.getTotalSize() << " " << m_HashMap.getUniqueSize();
                    break;}
                case 'S': {
                    string value;
                    MultiHashMap<double, string> m_HashMap;
                    for (int i = 0; i < N; ++i) {
                        F >> comand;
                        if (comand == 'A') {
                            F >> key >> value;
                            m_HashMap.Insert(key, value);
                        } else {
                            F >> key;
                            m_HashMap.remove(key);
                        }
                    }
                    cout << m_HashMap.getTotalSize() << " " << m_HashMap.getUniqueSize();
                    break;
                }
            }
            break;
        }
        case ('S'): {
            string key;
            switch (valueType) {
                case 'I':{
                    int value;
                    MultiHashMap<string, int> m_HashMap;
                    for (int i = 0; i < N; ++i) {
                        F >> comand;
                        if (comand == 'A') {
                            F >> key >> value;
                            m_HashMap.Insert(key, value);
                        } else {
                            F >> key;
                            m_HashMap.remove(key);
                        }
                    }
                    cout << m_HashMap.getTotalSize() << " " << m_HashMap.getUniqueSize();
                    break;}
                case 'D':{
                    double value;
                    MultiHashMap<string , double> m_HashMap;
                    for (int i = 0; i < N; ++i) {
                        F >> comand;
                        if (comand == 'A') {
                            F >> key >> value;
                            m_HashMap.Insert(key, value);
                        } else {
                            F >> key;
                            m_HashMap.remove(key);
                        }
                    }
                    F >> key;
                    double* a = m_HashMap.GetAllElements(key);
                    if(a == nullptr){
                        cout << a[0];
                    }
                    if(a != nullptr){
                        int n = m_HashMap.GetNumberEl(key);
                        cout << n << ": ";
                        for (int i = 0; i < n; ++i) {
                            cout << a[i+1] <<' ';
                        }
                    }
                    break;
                }
                case 'S': {
                    string value;
                    MultiHashMap<string, string> m_HashMap;
                    for (int i = 0; i < N; ++i) {
                        F >> comand;
                        if (comand == 'A') {
                            F >> key >> value;
                            m_HashMap.Insert(key, value);
                        } else {
                            F >> key;
                            m_HashMap.remove(key);
                        }
                    }
                    cout << m_HashMap.getTotalSize() << " " << m_HashMap.getUniqueSize();
                    break;
                }
            }
            break;
        }
    }


    return 0;
}
