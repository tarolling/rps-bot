const { defaultTimeout } = require('../config/settings.json');

class Game {
    rank;
    id;
    playerOne;
    playerTwo;
    gameNum;
    timeout;

    constructor(rank) {
        this.rank = rank;
        this.id = global.lobbyId;
        this.playerOne;
        this.playerTwo;
        this.gameNum = 0;
        this.timeout;
    }

    setTimeout(msecs) {
        this.timeout = setTimeout(() => {
            delete this;
        }, msecs || defaultTimeout);
    }

    clearTimeout() {
        
    }
}

module.exports = Game;