#pragma warning(disable : 4996)
#include <fstream>
#include <Windows.h>
#include <iostream>
#include <io.h>

enum var_case { print = 1, add_student, delete_student, seacher_student, change_info_stident, close_f };
enum var_case_ch { ch_id_gr = 1, ch_surname, ch_name, ch_patronymic, ch_all };

struct Zap {
    int id_zachet = 0;
    int id_gr;
    char surname[20];
    char name[20];
    char patronymic[30];
};

struct Block {
    Zap zap_block[5];
    int Next_Block = NULL;
    int Prev_Block = NULL;
};
struct Catalog_block {
    int first = NULL;
    int last = NULL;
};

struct Null_Block {
    int size_block = sizeof(Block);
    int size_Zap = sizeof(Zap);
    int count_Zap_in_block = 5;
    Catalog_block Catalog[4];
};




struct For_search_Block 
{
    Block block;
    int namber_zap;
    int namber_block;
};

//==============================

For_search_Block search_Block(int key, char name_file[], Null_Block* null_block);
void search(int key, char name_file[], Null_Block* null_block);
void delete_Zap_in_Block(Zap* zap, char name_file[], Null_Block* null_block);
int write_block(Block* this_block, int Namber_Block, char name_file[], Null_Block* null_block);
int read_block(Block* this_block, int Namber_Block, char name_file[], Null_Block* null_block);
void add_Zap_in_Block(Zap* zap, char name_file[], Null_Block* null_block);
int write_null_block(char name_file[], Null_Block* null_block);


int write_null_block(char name_file[], Null_Block* null_block)
{
    FILE* outfile;

    outfile = fopen(name_file, "rb+");
    if (outfile == NULL) 
    {
        fprintf(stderr, "\nError opened file\n");
        return -1;
    }

    fseek(outfile, 0, SEEK_SET);
    int flag = fwrite(null_block, sizeof(Null_Block), 1, outfile);
    printf("Внесены изменения в каталог бакетов");
    if (!flag)
    {
        printf("NO");
        return -2;
    }
    fclose(outfile);
}






int write_block(Block* this_block, int Namber_Block, char name_file[], Null_Block* null_block) 
{  //запись блока в определенную позицию файла с начала
    FILE* outfile;
    int size_block = null_block->size_block;
    outfile = fopen(name_file, "rb+");
    if (outfile == NULL) 
    {
        fprintf(stderr, "\nError opened file\n");
        return -1;
    }

    fseek(outfile, sizeof(struct Null_Block) + size_block * (Namber_Block - 1), SEEK_SET);
    int flag = fwrite(this_block, size_block, 1, outfile);

    if (!flag)
    {
        printf("NO");
        return -2;
    }
    fclose(outfile);
}

int read_block(Block* this_block, int Namber_Block, char name_file[], Null_Block* null_block)
{
    FILE* outfile;
    int size_block = null_block->size_block;
    outfile = fopen(name_file, "rb+");
    if (outfile == NULL)
    {
        fprintf(stderr, "\nError opened file\n");
        return -1;
    }
    fseek(outfile, sizeof(struct Null_Block) + size_block * (Namber_Block - 1), SEEK_SET);
    int flag = fread(this_block, size_block, 1, outfile);
    fclose(outfile);
    return flag;
}



int HASH(Zap* zap) 
{
    return zap->id_zachet % 4;
}

int HASH(Zap zap) 
{
    return zap.id_zachet % 4;
}

int HASH(int id_zachet)
{
    return id_zachet % 4;
}


//переделано под бакеты
void add_Zap_in_Block(Zap* zap, char name_file[], Null_Block* null_block) 
{
    Block this_block;
    bool flag = true;
    int namber_block = 1;
    FILE* outfile;
    int size_block = null_block->size_block;
    outfile = fopen(name_file, "rb+");
    fseek(outfile, 0, SEEK_END);
    int _file_size = ftell(outfile) - sizeof(Null_Block);
    int count_block = _file_size / null_block->size_block;
    int hash = HASH(zap);
    Catalog_block Backet  = null_block->Catalog[hash];

    if (Backet.first == NULL) 
    {  //если нет блоков в бакете, то создаём
        this_block.zap_block[0] = *zap;
        write_block(&this_block, count_block + 1, name_file, null_block);
        null_block->Catalog[hash].first = count_block + 1;
        null_block->Catalog[hash].last = count_block + 1;
        write_null_block(name_file, null_block);
        return;

    }


    For_search_Block result_search = search_Block(zap->id_zachet, name_file, null_block);

    if (result_search.namber_zap != -1) //если уже существует запись, она повторно не записывается.
    {
        printf("\n Запись уже существует");
        return;
    }
    this_block = result_search.block;
    namber_block = result_search.namber_block;


    for (int i = 0; i < null_block->count_Zap_in_block; i++) 
    {   //поиск пустой записи в последнем блоке бакета и при наличии запись в неё 
        
        if (this_block.zap_block[i].id_zachet == 0) 
        {

            this_block.zap_block[i] = *zap;
            write_block(&this_block, namber_block, name_file, null_block);
            return;
        }
    }
   
    Block New_block, Last_block;
    //создание нового блока в бакет, и перенос указателей 
    New_block.zap_block[0] = *zap;
    New_block.Prev_Block = null_block->Catalog[hash].last;

    read_block(&Last_block, null_block->Catalog[hash].last, name_file, null_block);
    Last_block.Next_Block = count_block + 1;
    null_block->Catalog[hash].last = count_block + 1;

    write_block(&New_block, count_block + 1, name_file, null_block);
    write_block(&Last_block, New_block.Prev_Block, name_file, null_block);
    write_null_block(name_file, null_block);
    
}

//не переделывалось под бакеты, т.к. в этом не нуждалось - работает на результате функции search_Block
void re_write_Zap_in_Block(int id_zachet, char name_file[], Null_Block* null_block) 
{
    Zap zap;
    Zap zap1;
    Block this_block;
    FILE* outfile;
    outfile = fopen(name_file, "rb+");

    For_search_Block result_search = search_Block(id_zachet, name_file, null_block);
    if (result_search.namber_zap == -1)
    {
        std::cout << "\nТакой записи не существует\n ";
        return;

    } //если не существует записи, то непроисходит запись
    this_block = result_search.block;
    zap = this_block.zap_block[result_search.namber_zap];
    int switch_on_change;
    std::cout << "\n Изменить в записи: 1- группу, 2 - фамилия, 3 - имя ,4 - отчество , 5 - всё(кроме зачётки) \n ";
    std::cin >> switch_on_change;

    while (!(std::cin)) 
    {
        std::cin.clear();
        std::cin.ignore();
        std::cout << "\nВведите номер группы\n ";
        std::cin >> switch_on_change;
    }


    switch (switch_on_change)
    {
    case(ch_id_gr):
    {
        int id_gr;
        std::cout << "\nВведите номер группы\n ";
        std::cin >> id_gr;

        while (!(std::cin)) 
        {
            std::cin.clear();
            std::cin.ignore();
            std::cout << "\nВведите номер группы\n ";
            std::cin >> id_gr;
        }

        zap.id_gr = id_gr;

        break;
    }
    case(ch_name): 
    {
        char name[20];
        std::cout << "\nВведите имя \n ";
        std::cin >> name;
        for (int i = 0; i < 20; i++)
        {
            zap.name[i] = name[i];
        }

        break;
    }
    case(ch_surname):
    {
        char surname[20];
        std::cout << "\nВведите фамилию \n ";
        std::cin >> surname;
        for (int i = 0; i < 20; i++) 
        {
            zap.surname[i] = surname[i];
        }


        break;


    }
    case(ch_patronymic): 
    {
        char patronymic[30];
        std::cout << "\nВведите  отчество\n ";
        std::cin >> patronymic;
        for (int i = 0; i < 30; i++) 
        {
            zap.patronymic[i] = patronymic[i];
        }


        break;
    }
    case(ch_all):
    {
        std::cout << "\nВведите: ID-группы, имя, отчество, фамилию\n ";
        std::cin >> zap.id_gr;
        while (!(std::cin)) 
        {
            std::cin.clear();
            std::cin.ignore();
            std::cout << "\nВведите: ID-группы, имя, отчество, фамилию\n ";
            std::cin >> zap.id_gr;
        }

        std::cin >> zap.name;
        std::cin >> zap.patronymic;
        std::cin >> zap.surname;
        this_block = result_search.block;
        this_block.zap_block[result_search.namber_zap] = zap;

        break;
    }

    default:
        break;
    }
    this_block.zap_block[result_search.namber_zap] = zap;
    write_block(&this_block, result_search.namber_block, name_file, null_block);
}


void delete_Zap_in_Block(Zap* zap, char name_file[], Null_Block* null_block)
{
    Zap null_zap;
    Zap end_zap;
    Block this_block;
    Block prev_block;
    Block next_block;
    Block end_block;
    Block end_block_in_backet;
    int namber_block;
    int namber_end_block_in_backet;
    int namber_zap;
    FILE* outfile;
    outfile = fopen(name_file, "rb+");
    int hash = HASH(zap);
    For_search_Block result_search = search_Block(zap->id_zachet, name_file, null_block);
    if (result_search.namber_zap == -1) //если не существует записи, то непроисходит удаление
    {
        printf("Такой записи не существует");
        return;
    }


    namber_zap = result_search.namber_zap;
    this_block = result_search.block;
    namber_block = result_search.namber_block;


    fseek(outfile, 0, SEEK_END);
    int _file_size = ftell(outfile) - sizeof(Null_Block);
    int count_block = _file_size / null_block->size_block;
    /*
    1) сначала меняем удаляемую запись и последнию в последнем блоке бакета
    2) в зависимости от пустоты последнего блока бакета меняем или нет ссылки на предшедствующий ему и в нулевом блоке
    3)если блок оказался пустым необходимо поменять его с последним блоком в куче, меняя в том ссылки
    4)при необходимости обрезать файl
    */
    namber_end_block_in_backet = null_block->Catalog[hash].last;
    read_block(&end_block_in_backet, namber_end_block_in_backet, name_file, null_block);

    if (null_block->Catalog[hash].last == namber_block) //если удаление из последнего блока в бакете, то просто сдвигаем в нем записи
    {
        for (int i = namber_zap; i < null_block->count_Zap_in_block -2; i++)
        {
            end_block_in_backet.zap_block[i] = end_block_in_backet.zap_block[i + 1];
        }
        end_block_in_backet.zap_block[null_block->count_Zap_in_block-1] = null_zap;
    }
    else {//если удаление не последнего блока в бакете
        for (int i = null_block->count_Zap_in_block - 1; i > -1; i--)
        {
            if (end_block_in_backet.zap_block[i].id_zachet != 0)//ищем крайнюю не пустую запись в последнем блоке бакета и меняем с удаляемой
            {
                this_block.zap_block[namber_zap] = end_block_in_backet.zap_block[i];
                end_block_in_backet.zap_block[i] = null_zap;
                write_block(&this_block, namber_block, name_file, null_block);
                break;
            }
        }
    }
    if (end_block_in_backet.zap_block[0].id_zachet == 0) //если оказалось, что последний блок в бакете стал пустым
    {
        if (null_block->Catalog[hash].first == null_block->Catalog[hash].last) // если последний блок был единственным в бакете, то просто "обнуляем" ссылки бакета в нулевом блоке
        {
            null_block->Catalog[hash].first = NULL;
            null_block->Catalog[hash].last = NULL;
        }
        else  //если это не единственный блок в бакете, то меняем указатели в каталоге и предшествующем ему блоке
        {
            read_block(&prev_block, end_block_in_backet.Prev_Block, name_file, null_block);
            prev_block.Next_Block = NULL;
            write_block(&prev_block, end_block_in_backet.Prev_Block, name_file, null_block);
            null_block->Catalog[hash].last = end_block_in_backet.Prev_Block;

        }

        //Теперь меняем наш пустой блок бакета местами с последним блоком в куче
        read_block(&end_block, count_block, name_file, null_block);

        if (null_block->Catalog[HASH(end_block.zap_block[0])].first == count_block) //если последний блок в куче, является первым в его бакете, то только меняем указатель в каталоге
        {
            null_block->Catalog[HASH(end_block.zap_block[0])].first = namber_end_block_in_backet;
        }
        else //если последний блок в куче не является первым в своём бакете
        {
            read_block(&prev_block, end_block.Prev_Block, name_file, null_block);

            if (count_block != namber_end_block_in_backet) //если последний блок в куче не является блоком, который мы удаляем, то просто меняем ссылку
            {
                prev_block.Next_Block = namber_end_block_in_backet;
            }
            else//если последний блок в куче является блоком, который мы удаляем, то меняем ссылку у предыдущего блока (не делается изменение ссылки в каталоге, потому что это будет в коде ниже)
            {
                prev_block.Next_Block = NULL;
            }
            
            write_block(&prev_block, end_block.Prev_Block, name_file, null_block);
        }


        if (null_block->Catalog[HASH(end_block.zap_block[0])].last == count_block) //если последний блок в куче, является последним в своём бакете
        {
            if (count_block != namber_end_block_in_backet) //если мы удаляем не последний блок в куче, то перекидываем ссылки на удаляемый блок в каталоге
            {
                null_block->Catalog[HASH(end_block.zap_block[0])].last = namber_end_block_in_backet;
            }
            else //если мы удаляем  последний блок в куче, то перекидываем ссылки на блок предшествующий удаляемому 
            
            {
                null_block->Catalog[HASH(end_block.zap_block[0])].last = end_block.Prev_Block;
            }
        }
        else//если последний блок в куче, не является последним в своём бакете, то просто соединякм 
        {
            read_block(&next_block, end_block.Next_Block, name_file, null_block);
            next_block.Prev_Block = namber_end_block_in_backet;
            write_block(&next_block, end_block.Next_Block, name_file, null_block);
        }
        write_block(&end_block, namber_end_block_in_backet, name_file, null_block);

        int filedes = _fileno(outfile);
        fseek(outfile, 0, SEEK_END);
        int _file_size = ftell(outfile) - null_block->size_block;
        _chsize_s(filedes, _file_size);

    }

    else// если последний блок бакета не стал пустым, то просто его записываем
    {
        write_block(&end_block_in_backet, null_block->Catalog[hash].last, name_file, null_block);
    }

    write_null_block(name_file, null_block);
   
    printf("\nУдаление успешно\n");
}

//переделано под бакеты
For_search_Block search_Block(int key, char name_file[], Null_Block* null_block)
{
    For_search_Block result;
    int flag = 1;
    int count_Zap_in_Bloc = null_block->count_Zap_in_block;
    Block this_block;

    FILE* outfile;
    outfile = fopen(name_file, "rb+");

    if (outfile == NULL)
    {
        fprintf(stderr, "\nError opened file\n");
        return result;
    }
    int namber_block = 0;
    
    //добавить проверку на конец файла
    int hash = HASH(key);
    fseek(outfile, 0, SEEK_END);
    int _file_size = ftell(outfile) - sizeof(Null_Block);
    int count_block = _file_size / null_block->size_block;

    result.block = this_block;
    result.namber_zap = -1;
    result.namber_block = count_block;
    Catalog_block Backet = null_block->Catalog[hash];

    if (Backet.first == NULL) 
    { //если нет блоков в бакете, то нет и записи
            return result;
    }
   
    read_block(&this_block, Backet.first, name_file, null_block);
    result.block = this_block;
    namber_block = Backet.first;
    while (this_block.Next_Block != NULL) 
    { //проходимся в цикле по блокам в бакете и ищем запись, возвращаем при удаче блок и информацию о нёс и найденной записи 
        for (int i = 0; i < null_block->count_Zap_in_block; i++)
        {
            if (key == this_block.zap_block[i].id_zachet) 
            {
                result.block = this_block;
                result.namber_block = namber_block;
                result.namber_zap = i;
                return result;
            }
        }
        namber_block = this_block.Next_Block;
        read_block(&this_block, namber_block, name_file, null_block);
    } 
    //проверяем последний блок в бакете, при удаче возвращаем инф о найденной записи
    for (int i = 0; i < null_block->count_Zap_in_block; i++)
    {
        if (key == this_block.zap_block[i].id_zachet)
       {
            result.block = this_block;
            result.namber_block = namber_block;
            result.namber_zap = i;
            return result;
        }
    }


    //если запись не найдена, то возвращаем последний блок в бакете и помечаем разультат, как неудачу поиска
    result.block = this_block;
    result.namber_block = namber_block;
    result.namber_zap = -1;
    return result;

}

//переделано под бакеты
void search(char name_file[], Null_Block* null_block) 
{
    FILE* outfile;
    outfile = fopen(name_file, "rb+");
    fseek(outfile, 0, SEEK_END);
    int _file_size = ftell(outfile) - sizeof(Null_Block);
    int count_block = _file_size / null_block->size_block;

    if (count_block == 0)
    {
        std::cout << "\nЗаписей в файле нет\n ";
        return;
    }


    int id_zachet;
    std::cout << "\nВведите номер зачетки\n ";
    std::cin >> id_zachet;

    while (!(std::cin))
    {
        std::cin.clear();
        std::cin.ignore();
        std::cout << "\nВведите номер зачетки\n ";
        std::cin >> id_zachet;
    }

    int key = id_zachet;
    bool flag = false;
    int count_Zap_in_Bloc = null_block->count_Zap_in_block;
    Block this_block;
    For_search_Block result;

    if (outfile == NULL)
    {
        fprintf(stderr, "\nError opened file\n");
    }

    int namber_block = 1;
    result = search_Block(key, name_file, null_block);

    if (result.namber_zap == -1) 
    {
        std::cout << "\nТакого студента нет\n ";
        return;
    }

    this_block = result.block;
    int namber_zap = result.namber_zap;

    if (result.namber_zap != -1) 
    {
        std::cout << this_block.zap_block[namber_zap].id_zachet << " " << this_block.zap_block[namber_zap].id_gr << " " << this_block.zap_block[namber_zap].surname << " " << this_block.zap_block[namber_zap].name << " " << this_block.zap_block[namber_zap].patronymic << "\n";
        return;
    }

}


int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    Null_Block null_block;
    char name_file[50];// = "C:/Users/Azerty/Desktop/test4.txt";
    FILE* outfile;


    int switch_on = -1;

    std::cout << "Введите имя файла";
    std::cin >> name_file;


    outfile = fopen(name_file, "r+b");
    if (outfile == 0) 
    {
     
        fprintf(stderr, "\nСоздан новый файл\n");
        outfile = fopen(name_file, "a+b");
        fwrite(&null_block, sizeof(struct Null_Block), 1, outfile);
        fclose(outfile);
      

    }
    fread(&null_block, sizeof(struct Null_Block), 1, outfile);
    fclose(outfile);


    while (switch_on != close_f)
    {
      
        std::cout << " \n2 - add_student, \n3 - delete_student, \n4 - seacher_student, \n5 - change_info_stident, \n6 -  close\n";


        std::cin >> switch_on;
        while (!(std::cin)) 
        {
            std::cin.clear();
            std::cin.ignore();
            std::cout << "\n2 - add_student, \n3 - delete_student, \n4 - seacher_student, \n5 - change_info_stident, \n6 -  close\n";;
            std::cin >> switch_on;
        }




        switch (switch_on)
        {

        case(print):
        {
            Block this_block;
            outfile = fopen(name_file, "r+b");
           
            fseek(outfile, 0, SEEK_END);
            int _file_size = ftell(outfile) - sizeof(Null_Block);
            int count_block = _file_size / null_block.size_block;
            for (int i = 1 ;i<=count_block;i++)
            {
                std::cout << "Блок " << i << std::endl;
                read_block(&this_block, i, name_file, &null_block);
                for (int j = 0; j < null_block.count_Zap_in_block; j++) 
                {
                    std::cout << this_block.zap_block[j].id_zachet<< std::endl;
                }
               
            }

            break;
        }


            //C:/Users/Azerty/Desktop/test1.txt
        case(add_student): 
        {
            Zap new_student;
            std::cout << "\nВведите: номер зачетки, ID-группы, имя, отчество, фамилию\n ";
            std::cin >> new_student.id_zachet;

            while (!(std::cin)) 
            {
                std::cin.clear();
                std::cin.ignore();
                std::cout << "\nВведите: номер зачетки, ID-группы, имя, отчество, фамилию\n ";
                std::cin >> new_student.id_zachet;
            }


            if (new_student.id_zachet < 1)
            {
                std::cout << "\nНедопустимое значение. Значение должно быть больше 0\n ";
                break;
            }
            std::cin >> new_student.id_gr;
            while (!(std::cin)) 
            {
                std::cin.clear();
                std::cin.ignore();
                std::cout << "\nВведите номер группы\n ";
                std::cin >> new_student.id_gr;
            }

            std::cin >> new_student.name;
            std::cin >> new_student.patronymic;
            std::cin >> new_student.surname;

            add_Zap_in_Block(&new_student, name_file, &null_block);

            break;
        }

        case(delete_student): 
        {
            Zap id_zachet;
            std::cout << "\nВведите номер зачетки\n ";
            std::cin >> id_zachet.id_zachet;
            while (!(std::cin))
            {
                std::cin.clear();
                std::cin.ignore();
                std::cout << "\nВведите: номер зачетки\n ";
                std::cin >> id_zachet.id_zachet;
            }


            delete_Zap_in_Block(&id_zachet, name_file, &null_block);
            break;
        }
        case(seacher_student): 
        {

            search(name_file, &null_block);
            break;
        }
        case(change_info_stident):
        {
            int id_zachet;
            std::cout << "\nВведите номер зачетки\n ";
            std::cin >> id_zachet;
            while (!(std::cin)) 
            {
                std::cin.clear();
                std::cin.ignore();
                std::cout << "\nВведите: номер зачетки\n ";
                std::cin >> id_zachet;
            }

            re_write_Zap_in_Block(id_zachet, name_file, &null_block);

            break;
        }
        case(close_f):
        {
            
            return 0;
        }


        default:

            break;
        }
    }



}

