
FastMySql    

FastMySql class    

@package  OpenGallery http://github.com/par7133   
@author   Daniele Bonini <code@gaox.io>     
@version  1.01     
@phpver   5.6 to 7.3      
@access   public    
@note You have to declare in your "config.inc" file - or whatever file you    
use for the purpose, the following global constants:   
define('DB_HOST', "p:your.db.ip.address");    
define('DB_USER', "your_dbuser");    
define('DB_PASSWORD', "your_dbpassword");    
define('DB_DBNAME', "your_db_name");    
define('DB_CHARSET', "utf8");     

The class makes use of the following global functions:      
function array_dim($array)     
{     
  if (is_array(reset($array))) {     
    $retval = array_dim(reset($array)) + 1;     
  } else {      
    $retval = 1;      
  }     
      
  return $retval;        
}      
       
function array_keys_count(& $array)      
{      
  return count(array_keys($array));      
}     
      
This class makes use of the class FastErr, part of the same
project and developed by me http://github.com/par7133

For any need of software additions, plugins and improvements please write to <a href="mailto:info@5mode.com">info@5mode.com</a>  

To help please donate by clicking <a href="https://gaox.io/l/dona1">https://gaox.io/l/dona1</a> and filling the form.  

Feedback: <a href="code@gaox.io">code@gaox.io</a>

