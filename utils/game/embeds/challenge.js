const { footer } = require('../../../config/embeds.json');


module.exports = (interaction) => {
    return {
        color: null,
        title: 'New Challenger Approaching',
        description: `**${interaction.user.username}** has challenged you to a game of RPS!`,
        footer
    };
};