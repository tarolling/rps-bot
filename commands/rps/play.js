const game = require('../../utils/game');
const { challenge } = require('../../utils/embeds');
const { addPlayerToQueue, deleteQueue } = require('../../utils/manageQueues');

module.exports = {
    data: {
        name: 'play',
        description: 'Play RPS against a specific user.',
        options: [
            {
                type: 6,
                name: 'user',
                description: 'Specify the user you would like to play against.',
                required: true
            }
        ],
        default_member_permissions: (1 << 11) // SEND_MESSAGES
    },
    async execute(interaction) {
        const target = interaction.options.getUser('user');
        
        if (!target) return interaction.reply({ content: 'Unable to find user.', ephemeral: true });
        if (target.id === interaction.user.id) return interaction.reply({ content: 'You cannot challenge yourself.', ephemeral: true });
        if (target.bot) return interaction.reply({ content: 'You cannot challenge a bot.', ephemeral: true });
        
        let queue = addPlayerToQueue(interaction.user, 'challenge');
        let acceptBtn = {
            type: 'BUTTON',
            label: 'Accept',
            custom_id: 'Accept',
            style: 'SUCCESS',
            emoji: null,
            url: null,
            disabled: false
        };
        let declineBtn = {
            type: 'BUTTON',
            label: 'Decline',
            custom_id: 'Decline',
            style: 'DANGER',
            emoji: null,
            url: null,
            disabled: false
        }
        let row = {
            type: 'ACTION_ROW',
            components: [acceptBtn, declineBtn]
        };
        let sentMsg;

        try {
            await target.send({ embeds: [challenge(interaction)], components: [row] })
                .then(msg => sentMsg = msg);
            interaction.reply({ content: 'Challenge sent!', ephemeral: true });
        } catch (err) {
            console.error(err);
            return interaction.reply({ content: 'Unable to DM user.', ephemeral: true });
        }

        let filter = (i) => {
            i.deferUpdate();
            return i.user.id === target.id;
        };
        await sentMsg.awaitMessageComponent({ filter, componentType: 'BUTTON', time: 30000 })
            .then((i) => {
                acceptBtn.disabled = true;
                declineBtn.disabled = true;
                row.components = [acceptBtn, declineBtn];
                if (i.customId === 'Accept') {
                    sentMsg.edit({ components: [row] });
                    queue = addPlayerToQueue(target, 'challenge');
                    game(queue, interaction);
                } else {
                    sentMsg.edit({ content: 'Challenge declined.', embeds: [], components: [row], ephemeral: true });
                    interaction.followUp({ content: 'Challenge declined.', ephemeral: true });
                    deleteQueue('challenge', queue.lobby.id, false);
                }
            })
            .catch(() => {
                acceptBtn.disabled = true;
                declineBtn.disabled = true;
                row.components = [acceptBtn, declineBtn];
                sentMsg.edit({ content: 'Challenge declined.', embeds: [], components: [row], ephemeral: true });
                interaction.followUp({ content: 'Challenge declined.', ephemeral: true });
                deleteQueue('challenge', queue.lobby.id, false);
            });
    }
};