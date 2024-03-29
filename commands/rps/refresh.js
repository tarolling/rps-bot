const updateLeaderboards = require('../../utils/db/updateLeaderboards');


module.exports = {
    data: {
        name: 'refresh',
        description: 'Refresh the leaderboards.',
        options: [],
        default_member_permissions: (1 << 11) // SEND_MESSAGES
    },
    async execute(interaction) {
        try {
            await interaction.deferReply({ ephemeral: true });
            await updateLeaderboards(interaction);
        } catch (err) {
            console.error(err);
            return interaction.editReply({ content: 'An error occurred while trying to refresh the leaderboards.', ephemeral: true });
        }
    }
};