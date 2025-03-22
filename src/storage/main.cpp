

#include <exception>


int main(int argc, char** argv)
{
    try
    {

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}