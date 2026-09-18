#include <iostream>
constexpr double ECARTMINI = 0.001;
using namespace std;

double valeurAbsolu(const double& x) noexcept
{
    if(x>=0)
        return x;
    else
        return -x;
}

double miseAPuissance(const double& x,const int& n) noexcept
{
    if(n==0)
    {
        return 1;
    }
    else
    {
        return(miseAPuissance(x,n-1)*x);
    }
}

//fonction simplification de x(n+1)=x(n)-(f(x(n))/f'(x(n))).
double pasDeNewton(const double& dividende, const double& x,const int& n) noexcept
{
    return (1/static_cast<double>(n))*(x*(n-1) + dividende/static_cast<double>(miseAPuissance(x,n-1)));
}

double methodeDeNewton(const double&dividende,const int&exposant) noexcept
{
    double racineNieme;
    double memoRacineNieme=-1;
    racineNieme = dividende/exposant;
    while(valeurAbsolu(racineNieme-memoRacineNieme)>ECARTMINI )
    {
        memoRacineNieme = racineNieme;
        racineNieme = pasDeNewton(dividende,racineNieme,exposant);
    }

    return racineNieme;
}


int main()
{
    try
    {
    double dividende;
    int exposant;
    cout << "Selectionnez un nombre dont on extrait la racine n-ieme superieur a 1.00." << endl;
    cin >> dividende;
    cout << "Selectionnez un entier n pour la racine n-ieme superieur a 1." << endl;
    cin >> exposant;

    if(!cin.good() || dividende<1.00 || exposant < 1)
    {
        if(!cin.good())
            throw std::invalid_argument("Vous n'avez pas rentre de valeurs correctes. Fin du programme");
        else
            throw std::invalid_argument("Vous avez choisis une valeur inferieure a 1. Fin du programme.");
    }
    else
    {
        cout << "La racine " << exposant << "ieme de "<<dividende <<" vaut "<<methodeDeNewton(dividende,exposant) <<"."<<   endl;
    }
    }
    catch(std::invalid_argument &e)
    {
        cerr<<e.what()<<endl;
        return -1;
    }
    return 0;
}
