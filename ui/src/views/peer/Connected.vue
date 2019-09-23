<template>
  <div class="peer">
    <div class="player" v-for="p in peer_list">
      <div class="title">
        <img class="small-avatar" :src="p.avatar" />
        <div class="peer-name">{{p.nickname}}</div>
        <div v-if="p.show_type != 0" @click.stop="p.show_type = 0">
          &nbsp;
          <i class="small material-icons">expand_less</i>
          &nbsp;
        </div>

        <div v-if="p.show_type == 0" @click.stop="p.show_type = 1">
          <i class="small material-icons">info_outline</i>
          &nbsp;&nbsp;&nbsp;&nbsp;
        </div>
        <div v-if="p.show_type == 0" @click.stop="chat_with(p)">
          <i class="small material-icons">chat</i>
          &nbsp;&nbsp;
        </div>
      </div>

      <div v-if="p.show_type != 0" class="peer-panel">
        <img :src="p.avatar" />
        <div>{{p.signature}}</div>       
      </div>
    </div>
  </div>
</template>

<script>

import util from "@/common/util";

export default {
  name: "peer",
  props: {
    msg: String
  },
  created: async function() {
    this.$root.$on("peer_changed", this.refresh);
  },
  destroyed() {
    this.$root.$off("peer_changed", this.refresh);
  },
  mounted() {
    this.refresh();
  },
  data() {
    return {
      peer_list: [],
    };
  },
  computed: {
    address() {
      return ``;
    }
  },
  methods: {
    chat_with(p) {
      this.$router.push({ name: 'peer-chat', params: { tp: p } } );
    },
    refresh() {
      // console.log('peer refresh')
      const tmp_peer = [];
      for (const [k, v] of peers.entries()) {
        if (v.usr) {
          tmp_peer.push({
            id: k,
            show_type: 0,
            nickname: v.usr.nickname,
            avatar: v.usr.avatar,
            signature: v.usr.signature,
          });
        }
      }
      // for test
      // let a = util.gen_face();
      // for(let i = 0; i < 10; ++i){
      //   tmp_peer.push({
      //     id: i,
      //     show_type: 0,
      //     nickname: '张三',
      //     avatar: a,
      //     signature: '路边小草',
      //   });
      // }
      // console.log(`tmp_peer=${JSON.stringify(tmp_peer)}`)
      this.peer_list = tmp_peer;
    }
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>

.small-avatar {
  width: 2rem;
  height: 2rem;
}
.title {
  width: 100%;
  display: flex;
}

.peer-name {
  flex: 1;
}
.peer-panel {
  width: 100%;
  /* border: 1px red solid; */
  overflow-y: auto;
}
.player {
  width: 100%;
  display: flex;
  flex-flow: column;
  align-items: center;
  /* text-align: left; */
  background-color: rgb(177, 250, 250);
  margin: 0.3em 0;
  border: 2px inset black;
  /* overflow-y: auto; */
}

.peer {
  width: 100%;
  display: flex;
  /* justify-content: space-between; */
  align-items: center;
  flex-flow: column;
  /* overflow: hidden; */
  font-weight: 700;
}

img {
  width: 100%;
}

</style>
