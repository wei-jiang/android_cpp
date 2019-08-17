<template>
  <div class="settings" @change="save_settings">
    <div class="p-info">
      <div>{{$t('nickname')}}</div>
      <input maxlength="20" :placeholder="$t('nickname')" v-model="user.nickname" />
      <div>{{$t('avatar')}}</div>
      <img :src="user.avatar" />
      <input id="avatar-img" type="file" @change="process_file($event)">
      <button :disabled="requesting" @click="open_img('avatar-img')">{{$t('chg-img')}}</button>
      <div>{{$t('signature')}}</div>
      <textarea :placeholder="$t('signature')" maxlength="70" rows="3" v-model="user.signature"></textarea>
    </div>
    <div class="option">
      <fieldset>
        <legend>{{$t('video-chat')}}</legend>
        <input type="radio" id="v-allow" value="1" v-model="user.video_chat" />
        <label for="v-allow">{{$t('allow')}}</label>
        <br />
        <input type="radio" id="v-disallow" value="0" v-model="user.video_chat" />
        <label for="v-disallow">{{$t('disallow')}}</label>
      </fieldset>
      <fieldset>
        <legend>{{$t('audio-chat')}}</legend>
        <input type="radio" id="a-allow" value="1" v-model="user.audio_chat" />
        <label for="a-allow">{{$t('allow')}}</label>
        <br />
        <input type="radio" id="a-disallow" value="0" v-model="user.audio_chat" />
        <label for="a-disallow">{{$t('disallow')}}</label>
      </fieldset>
    </div>
    
  </div>
</template>

<script>
import _ from "lodash";
import util from "@/common/util";

export default {
  name: "settings",
  created: function() {},
  destroyed() {},
  mounted() {
    this.user = db.user.findOne({});
  },
  data() {
    return {
      user: {
        nickname: "",
        avatar: "",
        signature: "",
        video_chat: "1",
        audio_chat: "1"
      }
    };
  },
  computed: {
    store_url() {
      return "";
    }
  },
  methods: {
    save_settings() {
      db.user.update(this.user);
    },
    open_img(name) {
      $(`#${name}`).click();
    },
    process_file(event) {
      if (event.target.files.length == 0) return;
      let img_file = event.target.files[0];
      const imageType = /image.*/;
      const reader = new FileReader();
      if (!img_file.type.match(imageType)) {
        return util.show_alert_top_tm("请选择图片文件");
      }
      reader.onload = async e => {
        // resize dataUrl below 1M
        this.user.avatar = await util.resize_img_file(e.target.result);
   
      };
      reader.readAsDataURL(img_file);
    }
  }
};
</script>
<style scoped >
.settings {
  font-size: 1.2rem;
  min-height: 97%;
  position: relative;
  /* text-align: left; */
}
.p-info {
  width: 100%;
  border: 1px dashed;
}
button {
  font-size: 1.2rem;
  margin: 0.5em 0.5rem;
  background-color: rgb(165, 245, 227);
  border-radius: 0.9em;
  padding: 0.3em 1em;
}
input[type="file"] {
  display: none;
}
input,
textarea {
  flex: 1;
  font-size: 1.3rem;
  width: 99%;
}
.option{
  display: flex;
}
.option > fieldset{
  flex: 1;
}
input[type="radio"]{
  min-height: 1.1em;
  min-width: 1.1em;
  /* margin-left: 1.7em; */
}
img {
  width: 80%;
  /* text-align: center; */
}
</style>
