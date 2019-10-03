import Loki from 'lokijs';
import LokiIndexedAdapter from 'lokijs/src/loki-indexed-adapter';
const idbAdapter = new LokiIndexedAdapter();

import util from "@/common/util";
let db;

function populate_default() {
    if (!db.ui.findOne({})) {
        db.ui.insert({
            sort_type: 1,
            sort_asc: 'asc',
            audio_loop_type: '1'
        });
    }
    if (!db.svr.findOne({})) {
        db.svr.insert({
            http_port: 57000,
            socks_port: 57100,
        });
    }
    if (!db.ss.findOne({})) {
        db.ss.insert({
            addr: '139.155.50.166:57000',
            enabled: true
        });
    }
    if (!db.user.findOne({})) {
        db.user.insert({
            nickname: i18n.t('def_nickname'),
            avatar: util.gen_face(),
            signature: i18n.t('def_signature'),
            video_chat: '1',
            audio_chat: '1',
            proxy: '1',
            friend: '2',
        });
        // const t = db.user.findOne({})
        // console.log('-----------------' + JSON.stringify(t) + '------------------------')
    }
    // for test only
    // if ( !db.friends.findOne({}) ) {
    //     const zs = {
    //         nickname: '张三',
    //         avatar: util.gen_face(),
    //         signature: '隔壁小三'
    //     };
    //     db.friends.insert(zs);
    // }
    // if ( !db.blacklist.findOne({}) ) {
    //     const ls = {
    //         nickname: '李四',
    //         avatar: util.gen_face(),
    //         signature: '隔壁小四'
    //     };
    //     const ly = {
    //         nickname: '李一',
    //         avatar: util.gen_face(),
    //         signature: '隔壁小一'
    //     };
    //     db.blacklist.insert(ls);
    //     db.blacklist.insert(ly);
    // }
}
function clean_logs(){
    // console.log(`db.peer_chat_log.count()=${db.peer_chat_log.count()}`)
    if(db.peer_chat_log.count() > 2000){
        const n_del = db.peer_chat_log.count() - 2000;
        db.peer_chat_log.chain().find({}).limit(n_del).remove();
    }
    // console.log(`db.nearby_chat_log.count()=${db.nearby_chat_log.count()}`)
    if(db.nearby_chat_log.count() > 1000){
        const n_del = db.nearby_chat_log.count() - 1000;
        // console.log(`db.nearby_chat_log remove ${n_del} rows`)
        db.nearby_chat_log.chain().find({}).limit(n_del).remove();
    }
    // console.log(`db.world_chat_log.count()=${db.world_chat_log.count()}`)
    if(db.world_chat_log.count() > 1000){
        const n_del = db.world_chat_log.count() - 1000;
        db.world_chat_log.chain().find({}).limit(n_del).remove();
    }
}
//export promise?
export default new Promise((resolve, reject) => {
    if (db) {
        resolve(db);
    } else {
        let mgrDB = new Loki("pyStore.db", {
            adapter: idbAdapter,
            autoload: true,
            autoloadCallback: () => {
                db = {
                    user: mgrDB.getCollection("user") ? mgrDB.getCollection("user") : mgrDB.addCollection("user"),
                    friends: mgrDB.getCollection("friends") ? mgrDB.getCollection("friends") : mgrDB.addCollection("friends"),
                    blacklist: mgrDB.getCollection("blacklist") ? mgrDB.getCollection("blacklist") : mgrDB.addCollection("blacklist"),
                    ui: mgrDB.getCollection("ui") ? mgrDB.getCollection("ui") : mgrDB.addCollection("ui"),
                    svr: mgrDB.getCollection("svr") ? mgrDB.getCollection("svr") : mgrDB.addCollection("svr"),
                    ss: mgrDB.getCollection("ss") ? mgrDB.getCollection("ss") : mgrDB.addCollection("ss"),
                    // id, nickname, content, type, [img, text], dt, dir[0, 1]. where id is chat target id
                    peer_chat_log: mgrDB.getCollection("peer_chat_log") ? mgrDB.getCollection("peer_chat_log") : mgrDB.addCollection("peer_chat_log"),
                    // id is always refer to source
                    nearby_chat_log: mgrDB.getCollection("nearby_chat_log") ? mgrDB.getCollection("nearby_chat_log") : mgrDB.addCollection("nearby_chat_log"),
                    world_chat_log: mgrDB.getCollection("world_chat_log") ? mgrDB.getCollection("world_chat_log") : mgrDB.addCollection("world_chat_log"),
                }
                populate_default();
                clean_logs();
                resolve(db);
            },
            autosave: true,
            autosaveInterval: 1000
        });
    }
})