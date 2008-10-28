/** \file parameterhandler.hh
    \brief  containing class ParameterHandler
 **/

#ifndef PARAMETERHANDLER_HH
#define PARAMETERHANDLER_HH

#include <fstream>
#include <ostream>
#include <map>
#include "stuff.hh"
#include "logging.hh"

/**
 *  \brief class processing parameter file
 *
 *  \c ParameterHandler reads a parameter file once and stores all found values internally
 **/
class ParameterHandler
{
    private:
        typedef std::map< std::string, std::string > MapType;
        MapType parameter_map_;
        bool status_;

    public:
        /** \ Please doc me!
         *
         **/
        ParameterHandler( const std::string filename )
            :status_( false )
        {
            ParseParamFile( filename );
        }

        ParameterHandler(  )
            :status_( false )
        {
        }

        /** \brief function used for parametrized ctor and two-step creation
        **/
        bool ParseParamFile( const std::string filename )
        {
            std::ifstream parameter_file( filename.c_str() );
            if( parameter_file.is_open() )
            {
                while( !parameter_file.eof() )
                {
                    std :: string line;
                    std :: getline( parameter_file, line );

                    if( line.size() == 0 )
                        continue;

                    if( (line[ 0 ] != '%') && (line[ 0 ] != '#') )
                    {
                        std::string param_name = line.substr( 0, line.find_first_of(":") );
                        std::string value = line.substr( line.find_first_of(":")+1, line.length() );
                        parameter_map_[param_name] = value;
                    }

                }
                status_ = true;
                parameter_file.close();
            }
            else {
                //LOGERROR
                status_ = false;
                std::cerr << "ERROR: file " << filename << " not found!\n";
            }
            return Ok();
        }

        /** \todo Please doc me! */
        template < class ReturnType >
        ReturnType GetParameter(const std::string name) const{
            assert( status_ );
            MapType::const_iterator it = parameter_map_.find( name ) ;
            if ( it != parameter_map_.end() ){
                return Stuff::fromString<ReturnType>( it->second );
            }
            else {
                //LogError
                return (ReturnType) 0;
            }

        }

        /** \todo Please doc me! */
        void Print( LogStream &out ) const
        {
            assert( status_ );
            for (MapType::const_iterator it = parameter_map_.begin(); parameter_map_.end() != it; ++it){
                out << it->first << ":" << it->second << "\n" ;
            }
            //out << std::endl;
        }

        /** \todo Please doc me! */
        bool Ok()
        {
            return status_;
        }

        /** \todo Please doc me! */
        ~ParameterHandler(){}

};

/** \brief global singelton for paramhandler
**/
ParameterHandler& params()
{
    static ParameterHandler param;
    return param;
}

/**
 *  \brief class containing global parameters
 *
 *  \c ParameterContainer contains all the needed global parameters
 **/
class ParameterContainer
{
    public:
        /**
         *  \brief  constuctor
         *  \todo   implement + doc me
         **/
        ParameterContainer()
        {
        }

        /**
         *  \brief  destuctor
         *  \todo   implement + doc me
         **/
        ~ParameterContainer(){}

        /**
         *  \brief  prints all parameters
         *  \todo   implement me
         *  \arg    std::ostream &out stream to print out
         **/
        void Print( std::ostream &out ) const
        {
        }

         /**
         *  \brief  checks command line parameters
         *  \arg    int argc   number of command line arguments
         *  \arg    char** argv array of command line arguments
         *  \return bool returns true, if comman line arguments are valid
         **/
       bool ReadCommandLine( int argc, char** argv )
        {
            if ( argc == 2 )
            {
                parameter_filename_ = argv[1];
                return true;
            }
            else
            {
                std::cerr << "\nUsage: " << argv[0] << " parameterfile\n" << std::endl;
                return false;
            }
        }

        /**
         *  \brief  returns the filename of the parameterfile
         *  \todo   doc me
         *  \return std::string filename of the parameterfile
         **/
        std::string parameterFilename() const
        {
            return parameter_filename_;
        }

    private:
        std::string parameter_filename_;
};

#endif // end of PARAMETERHANDLER.HH
