const playSeries = require('../../utils/game/playSeries');
const { challenge } = require('../../utils/game/embeds');
const { addPlayerToChallenge, createQueue } = require('../../utils/game/manageQueues');


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
        const { user } = interaction;

        const target = interaction.options.getUser('user');
        
        if (!target) return interaction.reply({ content: 'Unable to find user.', ephemeral: true });
        if (target.id === interaction.user.id) return interaction.reply({ content: 'You cannot challenge yourself.', ephemeral: true });
        if (target.bot) return interaction.reply({ content: 'You cannot challenge a bot.', ephemeral: true });
        
        const rankName = `challenge-${user.id}`;
        await createQueue(rankName);
        await addPlayerToChallenge(rankName, user);

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

        let challengeMessage;

        try {
            await target.send({ embeds: [challenge(interaction)], components: [row] })
                .then(msg => challengeMessage = msg);
            await interaction.reply({ content: 'Challenge sent!', ephemeral: true });
        } catch (err) {
            console.error(err);
            return interaction.reply({ content: 'Unable to DM user.', ephemeral: true });
        }

        const filter = i => i.user.id === target.id;

        const collector = challengeMessage.createMessageComponentCollector({ filter, time: 30000 });

        collector.on('collect', async (i) => {
            i.deferUpdate();
            collector.stop();
            acceptBtn.disabled = true;
            declineBtn.disabled = true;
            row.components = [acceptBtn, declineBtn];
            if (i.customId === 'Accept') {
                await challengeMessage.edit({ components: [row] });
                const queue = await addPlayerToChallenge(rankName, target);
                await playSeries(queue, interaction);
            } else {
                await challengeMessage.edit({ content: 'Challenge declined.', embeds: [], components: [row], ephemeral: true });
                await interaction.followUp({ content: 'Challenge declined.', ephemeral: true });
            }
        });

        collector.on('end', async (collected, reason) => {
            if (reason !== 'time') return;

            acceptBtn.disabled = true;  
            declineBtn.disabled = true;
            row.components = [acceptBtn, declineBtn];
            await challengeMessage.edit({ content: 'Challenge timed out.', embeds: [], components: [row], ephemeral: true });
            await interaction.followUp({ content: 'Challenge timed out.', ephemeral: true });
        });
    }
};