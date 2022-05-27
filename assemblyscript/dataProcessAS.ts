const CALC_RESULT_OFFSET = 0; //0-7 (f64)

let counter:i32 = 0;
let total:f64 = 0;
let aggrVolume:f64 = 5; // #aggregated data 
let dataReadyFlag:i32 = 0;

//Aggregate data and calculate average
export function calcWasm(data: i32): f64 {
    total += data;
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