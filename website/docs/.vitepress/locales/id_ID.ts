import { createRequire } from 'module'
import { defineConfig } from 'vitepress'

const require = createRequire(import.meta.url)
const pkg = require('vitepress/package.json')

export default defineConfig({
  lang: 'id-ID',
  description: 'Solusi root kernel-based untuk perangkat Android GKI.',

  themeConfig: {
    nav: nav(),

    lastUpdatedText: 'Update Terakhir',

    sidebar: {
      '/guide/': sidebarGuide()
    },

    socialLinks: [
      { icon: 'github', link: 'https://github.com/tiann/KernelSU' }
    ],

    footer: {
        message: 'Rilis Dibawah GPL3 License.',
        copyright: 'Copyright © 2022-present KernelSU Developers'
    },

    editLink: {
        pattern: 'https://github.com/tiann/KernelSU/edit/main/website/docs/:path',
        text: 'Edit Halaman ini di GitHub'
    }
  }
})

function nav() {
  return [
    { text: 'Guide', link: '/guide/what-is-kernelsu' },
    { text: 'Github', link: 'https://github.com/tiann/KernelSU' }
  ]
}

function sidebarGuide() {
  return [
    {
        text: 'Petunjuk',
        items: [
          { text: 'Apa itu KernelSU?', link: '/guide/what-is-kernelsu' },
          { text: 'Instalasi', link: '/guide/installation' },
          { text: 'Bagaimana cara buildnya?', link: '/guide/how-to-build' },
          { text: 'Integrasi untuk perangkat non-GKI', link: '/guide/how-to-integrate-for-non-gki'},
          { text: 'FAQ', link: '/guide/faq' },
        ]
    }
  ]
}
