<template>
  <div class="up-home">
    <div v-if="uploading" class="progressbar">
      <div v-bind:style="{width: uploading.progress}"></div>
      <div class="cap">{{`${progress_cap(uploading)}(${uploading.progress})`}}</div>
    </div>
    <input type="file" @change="processFile($event)">
    <button class="upload_btn" @click.prevent="open_file()" v-if="!uploading">{{$t('upload-home')}}</button>
  </div>
</template>

<script>
import _ from 'lodash'
import util from "@/common/util";
export default {
  name: "UpHome",
  props: {
    msg: String
  },
  created: async function() {},
  destroyed() {},
  mounted() {

  },
  data() {
    return {
      uploading: null,
    };
  },
  computed: {
    address() {
      return ``;
    }
  },
  methods: {
    progress_cap(f){
      return `${_.truncate(f.name, {'length': 7})}${util.formatFileSize(f.size)}`
    },
    open_file() {
      $('input[type="file"]').click();
    },
    upload_file(file) {
      let loaded = 0;
      let step = 1048576; //1024*1024; size of one chunk
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
          Buffer.from(buff, "binary"),
          Buffer.from([flag], "binary")
        ]);
        $.ajax({
          type: "POST",
          url: "/upload_home",
          // timeout: 3000,
          dataType: "text",
          contentType: "application/octet-stream",
          data: buff,
          processData: false
        })
          .done(r => {
            loaded += step; //increasing loaded which is being used as start position for next chunk
            if (loaded <= total) {
              // if file is not completely uploaded
              blob = file.slice(loaded, loaded + step); // getting next chunk
              reader.readAsBinaryString(blob); //reading it through file reader which will call onload again. So it will happen recursively until file is completely uploaded.
            } else {
              // if file is uploaded completely
              loaded = total; // just changed loaded which could be used to show status.
              this.uploading = null;
            }
            if(this.uploading){
              this.uploading.progress = `${parseFloat(
                (loaded / total) * 100
              ).toFixed(2)}%`;
            }
          })
          .fail(err => {
            console.log("failed: ", err);
          });
      };
    },
    processFile(event) {
      const file_type = /zip.*/;
      if (event.target.files.length == 0) return;
      let f = event.target.files[0];
      console.log(`f.type=${f.type}`);
      if (!f.type.match(file_type)) {
        return util.show_alert_top_tm('请选择zip文件');
      }
      this.uploading = { progress: "0%", size: f.size, name: f.name };
      this.upload_file(f);
      $('input[type="file"]').val("");
    }
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
input[type="file"] {
  display: none;
}
.progressbar {
  position: relative;
  margin: 0.3em 0.5em;
  background-color: black;
  border-radius: 1em; /* (height of inner div) / 2 + padding */
  padding: 0.2em;
  overflow: hidden;
}
.progressbar > .cap {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translateX(-50%) translateY(-50%);
  background-color: initial;
  width: 100%;
  color: rgb(107, 0, 128);
  font-weight: bold;
}
.progressbar + .progressbar {
  margin-top: 0.7em;
}
.progressbar > div {
  text-align: center;
  background-color: rgb(166, 255, 0);
  height: 1.5em;
  line-height: 1.5em;
  border-radius: 0.7em;
}
.upload_btn {
  border-radius: 0.7em;
  display: block;
  background-color: rgb(166, 218, 218);
  color: rgb(71, 70, 70);
  font-weight: 900;
  font-size: 1.7rem;
  width: 90%;
  margin: 0.9em auto;
  text-align: center;
}
</style>
