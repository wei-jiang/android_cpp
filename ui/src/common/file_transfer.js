
import moment from "moment";
import _ from 'lodash'
import util from "./util";
import WSS from "@/wss";

window.TransAssort = {
  image:  0,
  audio:  1,
  video:  2,
  others: 3,
};
class FTrans {
  constructor(target_id, file, assort = TransAssort.others, cb) {
    this.cb = cb;
    this.send_file_to_peer(peers.get(target_id), file, assort)
  }
  send_file_to_peer(sp, file, assort) {
    let loaded = 0;
    let step = 16 * 1024; //16k size of one chunk
    let total = file.size; // total size of file
    let start = 0; // starting position
    let reader = new FileReader();
    let blob = file.slice(start, step); //a single chunk in starting of step size
    reader.readAsBinaryString(blob); // reading that chunk. when it read it, onload will be invoked

    reader.onload = e => {
      let flag = 0;
      if (loaded > 0 && loaded + step <= total) flag = 1;
      else if (loaded + step > total) flag = 2;
      let buff = reader.result;
      buff = Buffer.concat([
        Buffer.from(file.name),
        Buffer.from([0], "binary"),
        Buffer.from(file.type),
        Buffer.from([0], "binary"),
        Buffer.from([assort], "binary"),
        Buffer.from(buff, "binary"),
        Buffer.from([flag], "binary")
      ]);
      sp.send_buff(CMD.send_file, buff);
      vm.$once(`${file.name}_${flag}`, ()=>{
        loaded += step; //increasing loaded which is being used as start position for next chunk
          if (loaded <= total) {
            // if file is not completely uploaded
            blob = file.slice(loaded, loaded + step); // getting next chunk
            reader.readAsBinaryString(blob); //reading it through file reader which will call onload again. So it will happen recursively until file is completely uploaded.
          } else {
            // if file is uploaded completely
            loaded = total; // just changed loaded which could be used to show status.
            vm.$emit(`${file.name}_end`, '');
            // console.log(`FTrans emit: ${file.name}_end`);
            if(this.cb) this.cb();
          }
          const progress = `${parseFloat( (loaded / total) * 100 ).toFixed(2)}%`;
          // console.log(`FTrans emit: ${file.name}_progress, progress=${progress}`);
          vm.$emit(`${file.name}_progress`, progress);
      })
    };
  }

}


export default FTrans;