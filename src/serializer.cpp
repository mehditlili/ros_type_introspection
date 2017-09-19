#include <functional>
#include <ros_type_introspection/serializer.hpp>
#include <ros_type_introspection/deserializer.hpp>

namespace RosIntrospection{
  
// helper function to deserialize raw memory
using byte = uint8_t;
template <typename T> inline bool to_bytes( const T& object , std::vector<uint8_t> &buffer, std::vector<uint8_t> &true_buffer)
{
    std::array< byte, sizeof(T) > bytes ;
    const byte* begin = reinterpret_cast< const byte* >( std::addressof(object) ) ;
    const byte* end = begin + sizeof(T) ;
    std::copy( begin, end, std::begin(bytes) ) ;
    std::cout << "Buffer old size " << buffer.size() << std::endl;
    buffer.insert(buffer.end(), bytes.begin(), bytes.end());
    std::cout << "Buffer new size " << buffer.size() << std::endl;

    std::vector<uint8_t> tr_buffer(&true_buffer[0],&true_buffer[buffer.size()]);
    return buffer == tr_buffer;
}
  
  
bool to_bytes( const SString& object , std::vector<uint8_t> &buffer, std::vector<uint8_t> &true_buffer)
{
    std::vector< byte> bytes ;
    bytes.resize(object.size());
    const byte* begin = reinterpret_cast< const byte* >( std::addressof(object) ) ;
    const byte* end = begin + object.size() ;
    std::copy( begin, end, std::begin(bytes) ) ;
        std::cout << "String Buffer old size " << buffer.size() << std::endl;

    buffer.insert(buffer.end(), bytes.begin(), bytes.end());
        std::cout << "String Buffer new size " << buffer.size() << std::endl;

    std::vector<uint8_t> tr_buffer(&true_buffer[0],&true_buffer[buffer.size()]);
    return buffer == tr_buffer;
}

// parses the message definition and creates an empty message (having default values)
void gen_message_skeleton(const ROSTypeList& type_map,
			  ROSType type,
			  SString prefix,
			  ROSTypeFlat* flat_container_output)
{
  
  uint8_t buffer[8] = {0};
  buildRosFlatType(type_map,
		   type,
		   prefix,
		   buffer,
		   flat_container_output,
		   100
		  );
  
  // Transform result into a map for dict style access
  std::map<SString, boost::any> value_map;
  std::vector<SString> attribute_names;
  std::map<int, SString> order_map;
  SString tmp;
  for (int i= 0; i<flat_container_output->value.size(); i++)
  {
    flat_container_output->value.at(i).first.toStr(tmp);
    value_map.insert(std::make_pair(tmp, flat_container_output->value.at(i).second.auto_convert()));
    order_map.insert(std::make_pair(i, tmp));
    attribute_names.push_back(tmp);
  }
}

void fill_sub_array()
{
  
}
// set a value in the message, if the higher level arrays do not exist, create them
void set_message_attribute()
{
  
}

int32_t get_next_number(SString string_to_search, size_t pos)
{
  size_t begin, end;
  std::string tmp_str = string_to_search.toStdString().substr(pos, string_to_search.size());
  begin = tmp_str.find_first_of("0123456789", 0);
  if (begin != std::string::npos)
  {
    end = tmp_str.find_first_not_of("0123456789", begin);
    return atoi(tmp_str.substr(begin, end - begin).c_str());
  }
  else
    return -1;
}

void buildRosBuffer(std::map<SString, boost::any> value_map,
		    std::map<SString, BuiltinType> type_map,
		    std::map<int, SString> order_map,
		    std::vector<SString> attribute_names,
		    std::vector<uint8_t> &buffer,
		    std::vector<uint8_t> &true_buffer)
{
  std::string current_variable;
  std::vector<std::string> found_arrays;
  size_t pos, pos_const;
  bool is_similar;
  int32_t array_length;
  for (int i= 0; i<order_map.size(); i++)
  {
    // Handle beginning of arrays
    current_variable = order_map.at(i).toStdString();
    std::cout << current_variable << " : " << std::endl;
    // Check if there are arrays involved, : for const size arrays
    pos = current_variable.find(".0", 0);
    pos_const = current_variable.find(":0", 0);
    std::vector<uint16_t>idx_sequence;
    // This is used to not add arrays indexed by 0, they are just there to display the structure of the message
    if (pos != std::string::npos)
    {
      // Check if those arrays where already considered
      std::string array_name = current_variable.substr(0, pos);
      std::string array_content = current_variable.substr(pos, current_variable.size());
      if ((std::find(found_arrays.begin(), found_arrays.end(), array_name) == found_arrays.end()))
      {
	std::cout << "Going to check array defined at position " << pos << std::endl;
	found_arrays.push_back(array_name);
	//Find the length of that array
	for (std::vector<SString>::iterator iter = attribute_names.begin(); iter != attribute_names.end(); ++iter) {
	  size_t pos2 = (*iter).toStdString().find(array_name);
	  if ( pos2 != std::string::npos) 
	  {
	    array_length = get_next_number(*iter, pos);
	  }
	}
	std::cout << "Array length " << array_length << std::endl;
	//Append a buffer of 4 bytes to the buffer containing the length of the array
	//ONLY IF ARRAY HAS VARIABLE SIZE!!
	is_similar = to_bytes<int32_t>(array_length, buffer, true_buffer);
	
	/*if (!is_similar)
	{
	    std::cout << "PROBLEM INSERTING ARRAY SIZE for ARRAY " << current_variable << std::endl; 
	      for (int i = 0; i < buffer.size() ; i++) {
		std::cout  << std::hex << unsigned(true_buffer.at(i)) << " : " << std::hex << unsigned(buffer.at(i)) <<std::endl;
	      }
	    exit(1);
	}*/
      }
    }
    
    if (pos != std::string::npos || pos_const != std::string::npos)
    {
      // Skip arrays with 0 indices as they represent the structure of the message (that can be preserved if an empty messages is received)
      continue;
    }

    switch( type_map.at(order_map.at(i)))
    {
      
      case CHAR:
      case INT8:   is_similar = to_bytes<int8_t>(boost::any_cast<int8_t>(value_map.at(order_map.at(i))), buffer, true_buffer) ; break;

      case INT16:  is_similar = to_bytes<int16_t>(boost::any_cast<int16_t>(value_map.at(order_map.at(i))), buffer, true_buffer) ; break;
      case INT32:  is_similar = to_bytes<int32_t>(boost::any_cast<int32_t>(value_map.at(order_map.at(i))), buffer, true_buffer) ; break;
      case INT64:  is_similar = to_bytes<int64_t>(boost::any_cast<int64_t>(value_map.at(order_map.at(i))), buffer, true_buffer) ; break;

      case BOOL:
      case BYTE:
      case UINT8:  is_similar = to_bytes<uint8_t>(boost::any_cast<uint8_t>(value_map.at(order_map.at(i))), buffer, true_buffer) ; break;

      case UINT16:  is_similar = to_bytes<uint16_t>(boost::any_cast<uint16_t>(value_map.at(order_map.at(i))), buffer, true_buffer) ; break;
      case UINT32:  is_similar = to_bytes<int32_t>(boost::any_cast<uint32_t>(value_map.at(order_map.at(i))), buffer, true_buffer) ; break;
      case UINT64:  is_similar = to_bytes<int64_t>(boost::any_cast<uint64_t>(value_map.at(order_map.at(i))), buffer, true_buffer) ; break;

      case FLOAT32: is_similar = to_bytes<float>(boost::any_cast<float>(value_map.at(order_map.at(i))), buffer, true_buffer) ; break;
      case FLOAT64: is_similar = to_bytes<double>(boost::any_cast<double>(value_map.at(order_map.at(i))), buffer, true_buffer) ; break;
	
      case DURATION: 
      case TIME: { 
	double extr_time = boost::any_cast<double>(value_map.at(order_map.at(i)));
	ros::Time time_ros(extr_time);
	to_bytes<uint32_t>(time_ros.sec, buffer, true_buffer);
	to_bytes<uint32_t>(time_ros.nsec, buffer, true_buffer); break;
      }

      case STRING: {
	SString extr_str = boost::any_cast<SString>(value_map.at(order_map.at(i))); 
	to_bytes<int32_t>(extr_str.size(), buffer, true_buffer);
	to_bytes(extr_str, buffer, true_buffer) ; break;
      }
      
      default:  std::cout << "Invalid" <<std::endl; break;
    }

    if (!is_similar)
    {
	std::cout << "PROBLEM INSERTING Data for " << current_variable << std::endl; 
	for (int i = 0; i < buffer.size() ; i++) {
	  std::cout  << std::hex << unsigned(true_buffer.at(i)) << " : " << std::hex << unsigned(buffer.at(i)) <<std::endl;
	}
	exit(1);
    }
  }
  
}

} // end namespace RosIntrospection