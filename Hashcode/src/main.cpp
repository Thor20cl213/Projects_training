#include <iostream>
#include <algorithm>
#include <ctime>
#include <map>
#include <thread>
#include <future>
#define NOMBRE_OCCURENCE 3
#define TEMPS_LIMITE 2000

using namespace std;
class Collision;
static multimap<int,Collision> mapOfCollision;
class Collision
{
    int myInt;
    string myString;
public:

    Collision(const int& anInt,const string& aString):myInt(anInt),myString(aString){};

    //methode non membre
    friend int hash_value(const Collision& aClassElement)
    {
    return static_cast<int>(aClassElement.myString[0] + aClassElement.myInt);
    }
    const string& getString(){return myString;};
    const int& getInt(){return myInt;};
};

Collision random_Collision( size_t length )
{
    auto randchar = []() -> char
    {
        const char charset[] = "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );

    return Collision(rand(),str);
}
void rempliLaMap()
{
    Collision aCollision = random_Collision(20);
    mapOfCollision.insert(std::pair<int,Collision>((hash_value(aCollision)),aCollision));
}

int regardeLaMap()
{
    int aReturnValue = -1;
    for(auto it=mapOfCollision.begin();it!=mapOfCollision.end();it++)
    if(mapOfCollision.count((*it).first)==NOMBRE_OCCURENCE){
        aReturnValue = (*it).first;
    }
    return aReturnValue;
}

void afficheLesResultats(const int& idMap)
{
    for(auto itr = mapOfCollision.lower_bound(idMap); itr != mapOfCollision.upper_bound(idMap); itr++)
        cout << itr->first << " "<< itr->second.getInt() << " " << itr->second.getString() <<endl;
}

bool timeOut()
{
    static clock_t now = clock();
    return ((clock()- now)>TEMPS_LIMITE);
}

int main()
{
    srand(time(nullptr));
    try{
        int idMap = -1;
        while(idMap == -1)
        {
            std::future<bool> f = std::async(std::launch::async,timeOut);
            rempliLaMap();
            //std::future<void> remplissageMap = std::async(std::launch::async,rempliLaMap);
            std::future<int> regardMap = std::async(std::launch::async,regardeLaMap);
            idMap = regardMap.get();
            if(f.get())
                throw std::exception();
        }
        afficheLesResultats(idMap);
        return 0;
    }
    catch(std::exception &e)
    {
        cerr<<"TEMPS LIMITE DE "<<TEMPS_LIMITE<<" MILLISECONDES DEPASSE."<<endl;
        return -1;
    }
}
