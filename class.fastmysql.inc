<?php

/**
 * Copyright (c) 2016, 2022, the Open Gallery's contributors
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither Open Gallery nor the names of its contributors 
 *       may be used to endorse or promote products derived from this software 
 *       without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * class.fastmysql.inc
 * 
 * FastMySql class.
 *
 * @author Daniele Bonini <my25mb@aol.com>
 * @copyrights (c) 2016, 2022, the Open Gallery's contributors http://opengallery.media
 * @license https://opensource.org/licenses/BSD-3-Clause 
 * 
 */

namespace OpenGallery\OpenGallery;

/**
 * FastMySql
 *
 * FastMySql class
 *
 * @package  OpenGallery
 * @author   Daniele Bonini <my25mb@aol.com>
 * @version  1.01
 * @access   public
 * @note You have to declare in your "config.inc" file - or whatever file you
 * use for the purpose, the following global constants:
 * define('DB_HOST', "p:your.db.ip.address");
 * define('DB_USER', "your_dbuser");
 * define('DB_PASSWORD', "your_dbuser_password");
 * define('DB_DBNAME', "your_db_name");
 * define('DB_CHARSET', "utf8"); 
 * 
 * This class makes use of the class FastErr, part of the Open Gallery
 * project and developed by me http://github.com/par7133
 * 
 */
class FastMySql extends \mysqli
{
  /**
   * The static instance of FastMySql
   *  
   * @access private
   * @var FastMySql
   */
  private static $_instance = null;

  /**
   * The number of open transactions
   * 
   * @access private
   * @var integer
   */
  private $_numTransactions = 0;
  
  /**
   * Get the static instance of FastMySql
   */
  public static function &getInstance()
  {  
    if(!isset(self::$_instance)){
      
      // Set a new connection..
      self::$_instance = new FastMySql(DB_HOST, DB_USER, DB_PASSWORD, DB_DBNAME);
      
      if (self::$_instance->connect_error) {
        FastErr::trigger_error1(ERR::ERR_GENERAL, "Connection to the db failed with the following error: " . self::$_instance->connect_error, __FILE__,__LINE__);
      } 
    }  
    
    // Set the connection charset..
    self::$_instance->set_charset(DB_CHARSET);
    
    return self::$_instance;  
  }
  
  /**
   * Check if the static instance is set
   * 
   * @return bool
   */
  public static function issetInstance()
  {
    return isset(self::$_instance);
  }
  
  /**
   * Unset the static instance
   * 
   * @return void
   */
  public static function unsetInstance()
  {
    self::$_instance = null;
  }
  
  /**
   * Default constructor
   */
  private function __construct($host = null, $username = null, $passwd = "", $dbname = null, $port = null, $socket = null)
  {
    parent::__construct($host, $username, $passwd, $dbname, $port, $socket);
  }
  
  /**
   * Start a transaction
   * 
   * @param int $flags valid flags
   * @param string $name [optional] Savepoint name for the transaction
   * @return bool TRUE on success or FALSE on failure
   */
  public function begin_transaction($flags = null, $name = null) 
  {
    $this->_numTransactions++;
   
    if (!isset($flags)) {
      return parent::begin_transaction();
    } else if (!isset($name)) {
      return parent::begin_transaction($flags);
    }    
    return parent::begin_transaction($flags, $name);
  }

  /**
   * Commits the current transaction
   * 
   * @param int $flags A bitmask of MYSQLI_TRANS_COR_* constants
   * @param string $name If provided then COMMIT/*name* / is executed
   * @return bool TRUE on success or FALSE on failure
   */
  public function commit($flags = null, $name = null) 
  {

    if ($this->hasTransactionStarted()) {
      $this->_numTransactions--;
      
      if (!isset($flags)) {
        return parent::commit();
      } else if (!isset($name)) {
        return parent::commit($flags);
      }    
      return parent::commit($flags, $name);
    }
    
    return false;
  }

  /**
   * Check if has transaction opened
   * 
   * @return bool if has transaction open, true/false
   */
  public function hasTransactionStarted()
  {
    return ($this->_numTransactions > 0) ? true : false;   
  }
  
  /**
   * Rolls back current transaction
   * 
   * @param int $flags A bitmask of MYSQLI_TRANS_COR_* constants.
   * @param string $name If provided then ROLLBACK/*name* / is executed.
   * @return bool TRUE on success or FALSE on failure.
   */
  public function rollback($flags = null, $name = null) 
  {
  
    if ($this->hasTransactionStarted()) {
      $this->_numTransactions--;
      
      if (!isset($flags)) {
        return parent::rollback();
      } else if (!isset($name)) {
        return parent::rollback($flags);
      }    
      return parent::rollback($flags, $name);
    }
    
    return false;
  }
  
  /**
   * Make a delete
   * 
   * @param string $sSQL the SQL string
   * @param array $aParams the parameters in the format [["type", $param], .. ], eg. [["s",$q], .. ]
   * @return int the number of the affected rows
   */
  public function delete($sSQL, $aParams = [])
  {
    if (($aParams != []) && array_dim($aParams)!=2) {
      FastErr::trigger_error1(ERR::ERR_GENERAL, "Invalid array dimensions for the call to the store proc: " . array_dim($aParams), __FILE__, __LINE__);        
    }
    
    if (!($preparedStmt = $this->prepare($sSQL))) {
      $this->rollback();
      FastErr::trigger_error1(ERR::ERR_GENERAL, "Preparetion failed: (" . $this->errno . ") " . $this->error, __FILE__, __LINE__);        
    }

    // if the params array is not empty..
    if (!empty($aParams)) {
      // parse the the params array..
      $sTypes = "";
      $sParams = "";
      
      $i = 0;
      foreach ($aParams as &$param) {
        $sTypes .= $param[0];
        //last db class change:
        //undo
        //if ($param[0]==="s") {
        //end undo
        //and take away the following line
        if ($param[0]==="s" && is_string($param[1])) {
          $param[1] = $this->real_escape_string($param[1]);
        }
        $sParams .= ", \$aParams[$i][1]";
        $i++;
      }
      $sTypes = "\"" . $sTypes . "\""; 
      $sParams = substr($sParams, 1);     
    
      // Bind statement..
      if (!eval("return \$preparedStmt->bind_param($sTypes, $sParams);")) {
        $this->rollback();
        FastErr::trigger_error1(ERR::ERR_GENERAL, "Binding parameters failed: (" . $preparedStmt->errno . ") " . $preparedStmt->error, __FILE__, __LINE__);        
      }
    }
    
    // Execute statement..
    if (!$preparedStmt->execute()) {
      $this->rollback();
      FastErr::trigger_error1(ERR::ERR_GENERAL, "Execute failed: (" . $preparedStmt->errno . ") " . $preparedStmt->error, __FILE__, __LINE__);
    }
    $affectedRows=$this->affected_rows;

    $preparedStmt->close();
    
    return $affectedRows;
  }

  /**
   * Make a delete by Store Proc (Adapter method)
   * 
   * @param string $sSQL the SQL string
   * @param array $aParams the parameters in the format [["type", $param], .. ], eg. [["s",$q], .. ]
   * @return int the number of affected rows
   */
  public function delete_SP($sSQL, $aParams = [])
  {
    return $this->select($sSQL, $aParams, "affected_rows")[0];
  }  
  
  /**
   * Make an insert
   * 
   * @param string $sSQL the SQL string
   * @param array $aParams the parameters in the format [["type", $param], .. ], eg. [["s",$q], .. ]
   * @return int the insert id
   */
  public function insert($sSQL, $aParams = [])
  {
    if (($aParams != []) && array_dim($aParams)!=2) {
      FastErr::trigger_error1(ERR::ERR_GENERAL, "Invalid array dimensions for the call to the store proc: " . array_dim($aParams), __FILE__, __LINE__);        
    }
      
    if (!($preparedStmt = $this->prepare($sSQL))) {
      $this->rollback();
      FastErr::trigger_error1(ERR::ERR_GENERAL, "Preparetion failed: (" . $this->errno . ") " . $this->error, __FILE__, __LINE__);        
    }

    // if the params array is not empty..
    if (!empty($aParams)) {
      // parse the the params array..
      $sTypes = "";
      $sParams = "";
      
      $i = 0;
      foreach ($aParams as &$param) {
        $sTypes .= $param[0];
        //last db class change:
        //undo
        //if ($param[0]==="s") {
        //end undo
        //and take away the following line
        if ($param[0]==="s" && is_string($param[1])) {
          $param[1] = $this->real_escape_string($param[1]);
        }
        $sParams .= ", \$aParams[$i][1]";
        $i++;
      }
      $sTypes = "\"" . $sTypes . "\""; 
      $sParams = substr($sParams, 1);     
    
      // Bind statement..
      if (!eval("return \$preparedStmt->bind_param($sTypes, $sParams);")) {
        $this->rollback();
        FastErr::trigger_error1(ERR::ERR_GENERAL, "Binding parameters failed: (" . $preparedStmt->errno . ") " . $preparedStmt->error, __FILE__, __LINE__);        
      }
    }
    
    // Execute statement..
    if (!$preparedStmt->execute()) {
      $this->rollback();
      FastErr::trigger_error1(ERR::ERR_GENERAL, "Execute failed: (" . $preparedStmt->errno . ") " . $preparedStmt->error, __FILE__, __LINE__);
    }
    $insertID=$this->insert_id;

    $preparedStmt->close();
    
    return $insertID;
  }

  /**
   * Make an insert by Store Proc (Adapter method)
   * 
   * @param string $sSQL the SQL string
   * @param array $aParams the parameters in the format [["type", $param], .. ], eg. [["s",$q], .. ]
   * @return int the insert id
   */
  public function insert_SP($sSQL, $aParams = [])
  {
    //return $this->select($sSQL, $aParams, "insert_id")[0];
    
    $retval = $this->select($sSQL, $aParams);
    
    if (isset($retval[0])) {
      return $retval[0];
    } else {
      return $retval;
    }
  }
  
  /**
   * Make a select
   * 
   * @param string $sSQL the SQL string
   * @param array $aParams the parameters in the format [["type", $param], .. ], eg. [["s",$q]]
   * @param string $column (optional) the column to return 
   * @return array the result of the select
   */
  public function select($sSQL, $aParams = [], $column = null)
  {
    if (($aParams != []) && array_dim($aParams)!=2) {
      FastErr::trigger_error1(ERR::ERR_GENERAL, "Invalid array dimensions for the call to the store proc: " . array_dim($aParams), __FILE__, __LINE__);        
    }
    
    settype($aResults, "array");
    
    if (!($preparedStmt = $this->prepare($sSQL))) {
      $this->rollback();
      FastErr::trigger_error1(ERR::ERR_GENERAL, "Preparetion failed: (" . $this->errno . ") " . $this->error, __FILE__, __LINE__);        
    }

    // if the params array is not empty..
    if (!empty($aParams)) {
      // parse the the params array..
      $sTypes = "";
      $sParams = "";
      
      $i = 0;
      foreach ($aParams as &$param) {
        $sTypes .= $param[0];
        //last db class change:
        //undo
        //if ($param[0]==="s") {
        //end undo
        //and take away the following line
        if ($param[0]==="s" && is_string($param[1])) {
          $param[1] = $this->real_escape_string($param[1]);
        }
        $sParams .= ", \$aParams[$i][1]";
        $i++;
      }
      $sTypes = "\"" . $sTypes . "\""; 
      $sParams = substr($sParams, 1);     
    
      // Bind statement..
      if (!eval("return \$preparedStmt->bind_param($sTypes, $sParams);")) {
        $this->rollback();
        FastErr::trigger_error1(ERR::ERR_GENERAL, "Binding parameters failed: (" . $preparedStmt->errno . ") " . $preparedStmt->error, __FILE__, __LINE__);        
      }
    }
    
    // Execute statement..
    if (!$preparedStmt->execute()) {
      $this->rollback();
      FastErr::trigger_error1(ERR::ERR_GENERAL, "Execute failed: (" . $preparedStmt->errno . ") " . $preparedStmt->error, __FILE__, __LINE__);
    }
    
    $result = $preparedStmt->get_result();

    // Buffer result..
    $preparedStmt->store_result();
    
    // Loading data results in an array to return..
    $aResults = array();
    if ($result) {
      if ($result->num_rows > 0) {
        $aResults = $result->fetch_all(MYSQLI_ASSOC);

        // if the row contain only one field I return a onedimension array.. 
        foreach ($aResults as &$array) {
          if (array_keys_count($array)>1) {
            break;
          }  
          $count = count($aResults);
          foreach ($array as $key => $value) {
            if ($count>1) {
              $aResults2[] = $value;
            } else {
              // if the row contain only one field and one row, eg. SELECT COUNT() 
              // I return the name of the field..
              $aResults2[$key] = $value;
            }  
          }
        }

        // if requested, I return only the values of one column..
        if (isset($column)) {
          $aResults2 = array_column($aResults, $column);
        }
      } 
      $result->free();
    }
    $preparedStmt->close();
    
    if (isset($aResults2)) {
      return $aResults2;
    } else {
      return $aResults;
    }
  }
  
  /**
   * Make a select by Store Proc (Adapter method)
   * 
   * @param string $sSQL the SQL string
   * @param array $aParams the parameters in the format [["type", $param], .. ], eg. [["s",$q]]
   * @param string $column (optional) the column to return 
   * @return array the result of the select
   */
  public function select_SP($sSQL, $aParams = [], $column = null)
  {
    return $this->select($sSQL, $aParams, $column);
  }
  
  /**
   * Make an update
   * 
   * @param string $sSQL the SQL string
   * @param array $aParams the parameters in the format [["type", $param], .. ], eg. [["s",$q], .. ]
   * @return int the number of the affected rows
   */
  public function update($sSQL, $aParams = [])
  {
    if (($aParams != []) && array_dim($aParams)!=2) {
      FastErr::trigger_error1(ERR::ERR_GENERAL, "Invalid array dimensions for the call to the store proc: " . array_dim($aParams), __FILE__, __LINE__);        
    }
        
    if (!($preparedStmt = $this->prepare($sSQL))) {
      $this->rollback();
      FastErr::trigger_error1(ERR::ERR_GENERAL, "Preparetion failed: (" . $this->errno . ") " . $this->error, __FILE__, __LINE__);        
    }

    // if the params array is not empty..
    if (!empty($aParams)) {
      // parse the the params array..
      $sTypes = "";
      $sParams = "";
      
      $i = 0;
      foreach ($aParams as &$param) {
        $sTypes .= $param[0];
        //last db class change:
        //undo
        //if ($param[0]==="s") {
        //end undo
        //and take away the following line
        if ($param[0]==="s" && is_string($param[1])) {
          $param[1] = $this->real_escape_string($param[1]);
        }
        $sParams .= ", \$aParams[$i][1]";
        $i++;
      }
      $sTypes = "\"" . $sTypes . "\""; 
      $sParams = substr($sParams, 1);     
    
      // Bind statement..
      if (!eval("return \$preparedStmt->bind_param($sTypes, $sParams);")) {
        $this->rollback();
        FastErr::trigger_error1(ERR::ERR_GENERAL, "Binding parameters failed: (" . $preparedStmt->errno . ") " . $preparedStmt->error, __FILE__, __LINE__);        
      }
    }
    
    // Execute statement..
    if (!$preparedStmt->execute()) {
      $this->rollback();
      FastErr::trigger_error1(ERR::ERR_GENERAL, "Execute failed: (" . $preparedStmt->errno . ") " . $preparedStmt->error, __FILE__, __LINE__);
    }
    $affectedRows=$this->affected_rows;

    $preparedStmt->close();
    
    return $affectedRows;
  }

  /**
   * Make an update by Store Proc (Adapter method)
   * 
   * @param string $sSQL the SQL string
   * @param array $aParams the parameters in the format [["type", $param], .. ], eg. [["s",$q], .. ]
   * @return int the number of affected rows
   */
  public function update_SP($sSQL, $aParams = [])
  {
    $retval = $this->select($sSQL, $aParams, "affected_rows");
    
    if (isset($retval[0])) {
      return $retval[0];
    } else {
      return $retval;
    }
  }  
}
