<template>
  <div class="settings" @change="save_settings">
    <div class="p-info">
      <div>{{$t('nickname')}}</div>
      <input maxlength="20"  type="text" :placeholder="$t('nickname')" v-model="user.nickname" @change="noty_nickname"/>
      <div>{{$t('avatar')}}</div>
      <img :src="user.avatar" />
      <input id="avatar-img" type="file" @change="process_file($event)">
      <button :disabled="requesting" @click="open_img('avatar-img')">{{$t('chg-img')}}</button>
      <div>{{$t('signature')}}</div>
      <textarea :placeholder="$t('signature')" maxlength="70" rows="3" v-model="user.signature" @change="noty_signature"></textarea>
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
      <hr>
      <fieldset class="proxy">
        <legend>代理设置</legend>
        <input type="radio" id="p-allow" value="1" v-model="user.proxy" />
        <label for="p-allow">{{$t('allow')}}</label>
 
        <input type="radio" id="p-prompt" value="2" v-model="user.proxy" />
        <label for="p-prompt">提示</label>

        <input type="radio" id="p-disallow" value="0" v-model="user.proxy" />
        <label for="p-disallow">{{$t('disallow')}}</label>
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
        audio_chat: "1",
        proxy: "1",
      }
    };
  },
  computed: {
    store_url() {
      return "";
    }
  },
  methods: {
    noty_nickname(){
      for (const [k, v] of peers.entries()) {
          v.send_string(CMD.noty_nickname, this.user.nickname)
      }
    },
    noty_signature(){
      for (const [k, v] of peers.entries()) {
          v.send_string(CMD.noty_signature, this.user.signature)
      }
    },
    noty_avatar(){
      for (const [k, v] of peers.entries()) {
          v.send_string(CMD.noty_avatar, this.user.avatar)
      }
    },
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
        this.noty_avatar();
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
/* .proxy {

} */
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
input[type="text"],
textarea {
  flex: 1;
  font-size: 1.3rem;
  width: 99%;
}
.option{
  display: flex;
  flex-flow: wrap;
}
.option hr {
  /* width: 100%; */
  opacity: 0;
  flex-basis: 100%; height: 1em; margin: 0; border: 0;
}
.option > fieldset{
  flex: 1;
}
input[type="radio"]{
  min-height: 1.4rem;
  min-width: 1.4rem;
  margin: 1.7em;
}
img {
  width: 80%;
  /* text-align: center; */
}
</style>
