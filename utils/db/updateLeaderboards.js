const ranks = require('../../config/ranks.json');
const capitalize = require('../misc/capitalize');
const leaderboard = require('../embeds/leaderboard');

const MongoClient = require('mongodb').MongoClient;
// eslint-disable-next-line
const uri = process.env.DB_URI;


module.exports = async (interaction) => {
    const guild = interaction.guild;
    const leaderboardChannel = guild.channels.cache.find(channel => channel.name === 'leaderboards');
    const dbClient = new MongoClient(uri, { useNewUrlParser: true, useUnifiedTopology: true });

    try {
        await dbClient.connect();
        const collection = dbClient.db('rps').collection('players');
        await leaderboardChannel.messages.fetch({ limit: 6 }).then(messages => messages.forEach(message => message.delete()));

        for (let i = Object.keys(ranks).length - 1; i >= 0; i--) {
            const rank = Object.keys(ranks)[i];
            const query = { $and: [{ rank: capitalize(rank) }, { season_games: { $gt: 0 } }] };
            const players = await collection.find(query).sort({ elo: -1 }).limit(10).toArray();
            if (players.length === 0) continue;
            await leaderboardChannel.send({ embeds: [leaderboard(players)] });
        } 

        await interaction.editReply({ content: 'Leaderboards refreshed.' });
    } catch (err) {
        console.log(err);
    } finally {
        await dbClient.close();
    }
};