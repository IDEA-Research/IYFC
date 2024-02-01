

/*
 * In Rust, implementing a function for use by C language
 *
 * MIT License
 * Copyright 2023 The IDEA Authors. All rights reserved.
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

extern crate libc;
use concrete::{ConfigBuilder, generate_keys, set_server_key, FheUint4, ClientKey,ServerKey};
use concrete::prelude::*;
use libc::c_uint;
use std::ffi::{CString,CStr};


pub struct 
KeysInfo {
    pub client_key :ClientKey,
    pub server_key :ServerKey,
}

//In Rust, implementing a function for use by C language. add "pub extern "C"""

#[no_mangle]
pub extern "C" fn c_generate_keys()->*mut KeysInfo {
    let config = ConfigBuilder::all_disabled().enable_default_uint4().build();

    let (keys,server_keys) = generate_keys(config);

    let two_keys  = KeysInfo{
        client_key: keys,
        server_key: server_keys,
    };
    return Box::into_raw(Box::new(two_keys));
    
}

#[no_mangle]
pub extern "C" fn c_generate_client_key()->*mut ClientKey {
    let config = ConfigBuilder::all_disabled().enable_default_uint4().build();
    let client_key = ClientKey::generate(config);
    return Box::into_raw(Box::new(client_key));
    
}

#[no_mangle] 
pub unsafe extern "C" fn c_free_clientkey(p_keys:* mut ClientKey) {
    // Take the ownership back to rust and drop the owner 
    let _ = Box::from_raw(p_keys); 
}


#[no_mangle]
pub extern "C" fn c_generate_server_key(p_keys:&ClientKey)->*mut ServerKey {
    let server_kc = (*p_keys).generate_server_key();
    return Box::into_raw(Box::new(server_kc));
    
}

#[no_mangle] 
pub unsafe extern "C" fn c_free_serverkey(p_keys: * mut ServerKey) {
    // Take the ownership back to rust and drop the owner 
    let _ = Box::from_raw(p_keys); 
}

#[no_mangle]
pub extern "C" fn c_try_encrypt(p_keys:&ClientKey,plaint_uint:c_uint) ->*mut FheUint4 {
    //
    let en_uint = FheUint4::try_encrypt(plaint_uint,p_keys).unwrap();
    return Box::into_raw(Box::new(en_uint));
}

#[no_mangle] 
pub unsafe extern "C" fn c_free_fheuint2(p_: * mut FheUint4) {
    // Take the ownership back to rust and drop the owner 
    let _ = Box::from_raw(p_); 
}


#[no_mangle]
pub extern "C" fn c_executor_div(p_keys:&ServerKey, lhs:&FheUint4, rhs:&FheUint4) ->*mut FheUint4{
    //mutithread
    set_server_key((*p_keys).clone());
    let en_output = lhs / rhs;
    return Box::into_raw(Box::new(en_output));
}

#[no_mangle]
pub extern "C" fn c_executor_div_plain(p_keys:&ServerKey, lhs:&FheUint4, rhs: u8) ->*mut FheUint4{
    //mutithread

    set_server_key((*p_keys).clone());
    let en_output = lhs / rhs;
    return Box::into_raw(Box::new(en_output));
}

#[no_mangle]
pub extern "C" fn c_executor_plain_div(p_keys:&ServerKey, lhs: u8, rhs:&FheUint4) ->*mut FheUint4{
    //mutithread
    set_server_key((*p_keys).clone());
    let en_output = lhs / rhs;
    return Box::into_raw(Box::new(en_output));
}


#[no_mangle]
pub extern "C" fn c_executor_mul(p_keys:&ServerKey, lhs:&FheUint4, rhs:&FheUint4) ->*mut FheUint4{
    //mutithread
    set_server_key((*p_keys).clone());
    let en_output = lhs * rhs;
    return Box::into_raw(Box::new(en_output));
}

#[no_mangle]
pub extern "C" fn c_executor_mul_plain(p_keys:&ServerKey, lhs:&FheUint4, rhs: u8) ->*mut FheUint4{
    //mutithread
    set_server_key((*p_keys).clone());
    let en_output = lhs * rhs;
    return Box::into_raw(Box::new(en_output));
}



#[no_mangle]
pub extern "C" fn c_executor_add(p_keys:&ServerKey, lhs:&FheUint4, rhs:&FheUint4) ->*mut FheUint4{
    //mutithread
    set_server_key((*p_keys).clone());
    let en_output = lhs + rhs;
    return Box::into_raw(Box::new(en_output));
}

#[no_mangle]
pub extern "C" fn c_executor_add_plain(p_keys:&ServerKey, lhs:&FheUint4, rhs: u8) ->*mut FheUint4{
    //mutithread
    set_server_key((*p_keys).clone());
    let en_output = lhs + rhs;
    return Box::into_raw(Box::new(en_output));
}


#[no_mangle]
pub extern "C" fn c_executor_sub(p_keys:&ServerKey, lhs:&FheUint4, rhs:&FheUint4) ->*mut FheUint4{
    //mutithread
    set_server_key((*p_keys).clone());
    let en_output = lhs - rhs;
    return Box::into_raw(Box::new(en_output));
}

#[no_mangle]
pub extern "C" fn c_executor_sub_plain(p_keys:&ServerKey, lhs:&FheUint4, rhs:u8) ->*mut FheUint4{
    //mutithread
    set_server_key((*p_keys).clone());
    let en_output = lhs - rhs;
    return Box::into_raw(Box::new(en_output));
}

#[no_mangle]
pub extern "C" fn c_executor_plain_sub(p_keys:&ServerKey, lhs:u8, rhs:&FheUint4) ->*mut FheUint4{
    //mutithread
    set_server_key((*p_keys).clone());
    let en_output = lhs - rhs;
    return Box::into_raw(Box::new(en_output));
}

#[no_mangle]
pub extern "C" fn c_left_rotate(p_keys:&ServerKey, lhs:&FheUint4, rhs:u8) ->*mut FheUint4{
    //mutithread
    set_server_key((*p_keys).clone());
    let en_output = lhs<<rhs;
    return Box::into_raw(Box::new(en_output));
}

#[no_mangle]
pub extern "C" fn c_right_rotate(p_keys:&ServerKey, lhs:&FheUint4, rhs:u8) ->*mut FheUint4{
    //mutithread
    set_server_key((*p_keys).clone());
    let en_output = lhs>>rhs;
    return Box::into_raw(Box::new(en_output));
}
#[no_mangle]
pub extern "C" fn c_negate(p_keys:&ServerKey, lhs:&FheUint4) ->*mut FheUint4{
    //mutithread
    set_server_key((*p_keys).clone());
    let en_output = -lhs;
    return Box::into_raw(Box::new(en_output));
}




#[no_mangle]
pub extern "C" fn c_decrypt(p_keys:&ClientKey, en_result:&FheUint4) ->u8{
    //println!("in rust decrepyt");println!("in rust decrepyt");
    return (*en_result).decrypt(p_keys);

}

#[no_mangle]
pub extern "C" fn c_serialize_client_key(p_keys:&ClientKey) ->*mut i8{
        let serialized = serde_json::to_string(p_keys).unwrap();
        let c_string = CString::new(serialized).unwrap();
        return c_string.into_raw();
}

#[no_mangle] 
pub unsafe extern "C" fn c_free_cstring(p_: * mut i8) {
    // Take the ownership back to rust and drop the owner 
    let _ = Box::from_raw(p_);
}


#[no_mangle]
pub extern "C" fn c_deserialize_client_key(input: *mut i8) -> *mut ClientKey {
        //Delegate the Deallocation of Serialized Memory to C++
        //let c_string = unsafe{CString::from_raw(input)};
        let c_string = unsafe{CStr::from_ptr(input)};
        let serialized  = c_string.to_str().unwrap();
        let de_result: ClientKey = serde_json::from_str(serialized).unwrap();
        return Box::into_raw(Box::new(de_result));
}

#[no_mangle]
pub extern "C" fn c_serialize_server_key(p_keys:&ServerKey) ->*mut i8{
        let serialized = serde_json::to_string(p_keys).unwrap();
        let c_string = CString::new(serialized).unwrap();
        return c_string.into_raw();
}

#[no_mangle]
pub extern "C" fn c_deserialize_server_key(input: *mut i8) -> *mut ServerKey {
        //in C, Release c_string
        //let c_string = unsafe{CString::from_raw(input)};
        let c_string = unsafe{CStr::from_ptr(input)};
        let serialized  = c_string.to_str().unwrap();
        let de_result: ServerKey = serde_json::from_str(serialized).unwrap();
        return Box::into_raw(Box::new(de_result));
}

#[no_mangle]
pub extern "C" fn c_serialize_fheuint2(p_keys:&FheUint4) ->*mut i8{
        let serialized = serde_json::to_string(p_keys).unwrap();
        let c_string = CString::new(serialized).unwrap();
        return c_string.into_raw();
}

#[no_mangle]
pub extern "C" fn c_deserialize_fheuint2(input: *mut i8) -> *mut FheUint4 {
        //let c_string = unsafe{CString::from_raw(input)};
        let c_string = unsafe{CStr::from_ptr(input)};
        let serialized  = c_string.to_str().unwrap();
        let de_result: FheUint4 = serde_json::from_str(serialized).unwrap();
        return Box::into_raw(Box::new(de_result));
}





