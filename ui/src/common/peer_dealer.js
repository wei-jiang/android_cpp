import _ from 'lodash'
import moment from "moment";
class PDealer {
    constructor() {
        this.dealers = {}
        this.dealers['handshake'] = this.handshake.bind(this);
    }

    handle_msg(sp, data) {
        // data in buffer type
        try {
            if( data == 'ping') {
                console.log('recieved ping')
                sp.send('pong')
                return; //heartbeat
            }
            if( data == 'pong') {
                console.log('recieved pong')
                return; //heartbeat
            }
            data = JSON.parse(data)
            const d = this.dealers[data.cmd];
            if(d){
                d(sp, data);
            }
        } catch (error) {
            console.log('parse peer data error:' + data)
        }
    };

    handshake(sp, data) {
        sp.usr = data.usr;
        vm.$emit('peer_changed', '');     
        console.log('handshake done!')
    };
}

export default PDealer;