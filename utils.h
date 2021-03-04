// template <typename Iterator>
// std::string join(Iterator begin, Iterator end, char separator = '.')
// {
//     std::ostringstream o;
//     if(begin != end)
//     {
//         o << *begin++;
//         for(;begin != end; ++begin)
//             o  << separator << *begin;
//     }
//     return o.str();
// }
//  
// template <typename Container>
// std::string join(Container const& c, char separator = '.') // can pass array as reference, too
// {
//     using std::begin;
//     using std::end;
//     return join(begin(c), end(c), separator);
//     // not using std::... directly:
//     // there might be a non-std overload that wouldn't be found if we did
// }

void print_bytes( void *ptr, size_t size )
{
    //char *buf = (char*) ptr;
    unsigned char *p = (unsigned char*)ptr ;

    for( size_t i = 0; i < size; i++ )
    {
        printf( "%02hhX ", p[i] ) ;
    }
    printf( "\n" ) ;

    for( size_t i = 0; i < size; i++ )
    {
        for( short j = 7; j >= 0; j-- )
        {
            printf( "%d", ( p[i] >> j ) & 1 ) ;
        }
        printf(" ");
    }
    printf("\n");
}

char* toHex(void *ptr, size_t size) {
    // //char *buf = (char*) ptr;
    // unsigned char *p = (unsigned char*)ptr ;
    //
    // for( size_t i = 0; i < size; i++ ) {
    //     printf( "%02hhX ", p[i] ) ;
    // }
  return "hex"; 
}
