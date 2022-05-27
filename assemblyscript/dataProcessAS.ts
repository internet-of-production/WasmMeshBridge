const CALC_RESULT_OFFSET = 0; //0-7 (f64)

let counter:i32 = 0;
let total:f64 = 0;
let aggrVolume:f64 = 5; // #aggregated data 
let dataReadyFlag:i32 = 0;

function bytesToNum(num1:u8,num2:u8,num3:u8,num4:u8):i32{
    let convertedValue:i32= num4
    convertedValue = (convertedValue<<8) + num3
    convertedValue = (convertedValue<<8) + num2
    convertedValue = (convertedValue<<8) + num1
    return convertedValue;
  }

//Aggregate data and calculate average
export function calcWasm(b1: u8, b2:u8, b3:u8, b4:u8): f64 {
    let dataValue:i32 = bytesToNum(b1, b2, b3, b4);
    total += dataValue;
    counter++;
    if(counter>=aggrVolume){
        let result:f64 = total/aggrVolume;
        store<f64>(CALC_RESULT_OFFSET, result);
        counter = 0;
        total = 0;
        dataReadyFlag = 1;
        return load<f64>(CALC_RESULT_OFFSET);
    }
    else {
        return 0;
    }
  }

  //Store counter for the data
export function getDataReadyFlag(): u8 {
    if(dataReadyFlag==1){
        dataReadyFlag = 0;
        return 1;
    }
    else{
        return 0;
    } 
}