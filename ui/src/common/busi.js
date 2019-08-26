
import moment from "moment";
import _ from 'lodash'
import util from "./util";
import WSS from "@/wss";

window.sss = {}
class Busi {
    constructor() {
        _.bindAll(this, ['init', 'reg_evt', 'routine']);
        this.need_check_friends = new Date();
    }
    init() {
        this.reg_evt();
        this.go_pub();
        this.routine();
    }
    routine(){
        let span;
        setTimeout(()=>{
            for (let [id, sp] of peers) {
                span = moment.duration(moment().diff(moment(sp.activity))).asSeconds();
                // remove peer in blacklist
                const to_be_removed = db.blacklist.findOne({id})
                if(span > 6 || to_be_removed){
                    console.log('remove inactive peer: '+ id)
                    sp.destroy();
                    peers.delete(id);
                    vm.$emit('peer_changed', '');
                }
            }
            // check friends
            span = moment.duration(moment().diff(moment(this.need_check_friends))).asSeconds();
            if(span > 15){
                const ids = db.friends.find({}).filter( f=> !peers.has(f.id) ).map(f=>f.id);
                if(ids.length > 0){
                    _.each(sss, (v, k)=>{
                        v.check_friends(ids);
                    })
                }
                this.need_check_friends = new Date();
            }
            this.replenish();
            this.routine();
        }, 1500)
    }
    replenish(){
        _.each(sss, (v, k)=>{
            v.replenish();
        })
    }
    go_pub() {
        try {
            const addrs = util.ss_addrs();
            // console.log(`go_pub()------------${JSON.stringify(addrs)}`)
            addrs.forEach(s => {
                if(s.enabled){
                    sss[s.addr] = new WSS(s.addr)
                }               
            })
        } catch (err) {
            console.log(`go_pub() exception`)
        }
    }
    reg_evt() {
        vm.$on("add_ss", data => {
            const addr = data.addr;
            if( !sss.hasOwnProperty(addr) ){                
                sss[addr] = new WSS(addr)
            }            
        });
        vm.$on("del_ss", data => {
            const addr = data.addr;
            if( sss.hasOwnProperty(addr) ){
                sss[addr].destroy();
                delete sss[addr];
            }
        });

        vm.$on("send_p2p_msg", data => {
            // {id, type, content}
            const id = data.id;
            delete data.id;
            peers.get(id).send_json(CMD.send_p2p_msg, data)
        });
    }
}


export default new Busi;